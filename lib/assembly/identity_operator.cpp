// Copyright (C) 2011-2012 by the BEM++ Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.


#include "config_trilinos.hpp"

#include "identity_operator.hpp"

#include "assembly_options.hpp"
#include "discrete_dense_linear_operator.hpp"
#include "discrete_sparse_linear_operator.hpp"

#include "../common/types.hpp"
#include "../fiber/basis.hpp"
#include "../fiber/explicit_instantiation.hpp"
#include "../fiber/local_assembler_factory.hpp"
#include "../fiber/local_assembler_for_operators.hpp"
#include "../fiber/opencl_handler.hpp"
#include "../fiber/raw_grid_geometry.hpp"
#include "../grid/entity_iterator.hpp"
#include "../grid/grid.hpp"
#include "../grid/grid_view.hpp"
#include "../grid/mapper.hpp"
#include "../space/space.hpp"

#include <boost/type_traits/is_complex.hpp>
#include <stdexcept>
#include <vector>

#ifdef WITH_TRILINOS
#include <Epetra_FECrsMatrix.h>
#include <Epetra_LocalMap.h>
#include <Epetra_SerialComm.h>
#endif // WITH_TRILINOS

namespace Bempp
{

#ifdef WITH_TRILINOS
// Internal helper functions for Epetra
namespace
{

template <typename ValueType>
int epetraSumIntoGlobalValues(Epetra_FECrsMatrix& matrix,
                              const std::vector<int>& rowIndices,
                              const std::vector<int>& colIndices,
                              const arma::Mat<ValueType>& values);

// Specialisation for double -- no intermediate array is needed
template <>
inline int epetraSumIntoGlobalValues<double>(
        Epetra_FECrsMatrix& matrix,
        const std::vector<int>& rowIndices,
        const std::vector<int>& colIndices,
        const arma::Mat<double>& values)
{
    assert(rowIndices.size() == values.n_rows);
    assert(colIndices.size() == values.n_cols);
    return matrix.SumIntoGlobalValues(rowIndices.size(), &rowIndices[0],
                                      colIndices.size(), &colIndices[0],
                                      values.memptr(),
                                      Epetra_FECrsMatrix::COLUMN_MAJOR);
}

// Specialisation for float
template <>
inline int epetraSumIntoGlobalValues<float>(
        Epetra_FECrsMatrix& matrix,
        const std::vector<int>& rowIndices,
        const std::vector<int>& colIndices,
        const arma::Mat<float>& values)
{
    // Convert data from float into double (expected by Epetra)
    arma::Mat<double> doubleValues(values.n_rows, values.n_cols);
    std::copy(values.begin(), values.end(), doubleValues.begin());
    return epetraSumIntoGlobalValues<double>(
                matrix, rowIndices, colIndices, doubleValues);
}

// Specialisation for std::complex<float>.
// WARNING: at present only the real part is taken into account!
// This is sufficient as long as we provide real-valued basis functions only.
template <>
inline int epetraSumIntoGlobalValues<std::complex<float> >(
        Epetra_FECrsMatrix& matrix,
        const std::vector<int>& rowIndices,
        const std::vector<int>& colIndices,
        const arma::Mat<std::complex<float> >& values)
{
    // Extract the real part of "values" into an array of type double
    arma::Mat<double> doubleValues(values.n_rows, values.n_cols);
    // Check whether arma::real returns a view or a copy (if the latter,
    // this assert will fail)
    for (int i = 0; i < values.n_elem; ++i)
        doubleValues[i] = values[i].real();
    return epetraSumIntoGlobalValues<double>(
                matrix, rowIndices, colIndices, doubleValues);
}

// Specialisation for std::complex<double>.
// WARNING: at present only the real part is taken into account!
// This is sufficient as long as we provide real-valued basis functions only.
template <>
inline int epetraSumIntoGlobalValues<std::complex<double> >(
        Epetra_FECrsMatrix& matrix,
        const std::vector<int>& rowIndices,
        const std::vector<int>& colIndices,
        const arma::Mat<std::complex<double> >& values)
{
    // Extract the real part of "values" into an array of type double
    arma::Mat<double> doubleValues(values.n_rows, values.n_cols);
    for (int i = 0; i < values.n_elem; ++i)
        doubleValues[i] = values[i].real();
    return epetraSumIntoGlobalValues<double>(
                matrix, rowIndices, colIndices, doubleValues);
}

} // anonymous namespace
#endif

template <typename BasisFunctionType, typename ResultType>
IdentityOperator<BasisFunctionType, ResultType>::IdentityOperator(
        const Space<BasisFunctionType>& testSpace,
        const Space<BasisFunctionType>& trialSpace) :
    ElementaryLinearOperator<BasisFunctionType, ResultType>(testSpace, trialSpace) {
}

template <typename BasisFunctionType, typename ResultType>
bool IdentityOperator<BasisFunctionType, ResultType>::supportsRepresentation(
        AssemblyOptions::Representation repr) const
{
    return (repr == AssemblyOptions::DENSE ||
            repr == AssemblyOptions::SPARSE ||
            repr == AssemblyOptions::ACA);
}

template <typename BasisFunctionType, typename ResultType>
std::auto_ptr<DiscreteLinearOperator<ResultType> >
IdentityOperator<BasisFunctionType, ResultType>::assembleWeakForm(
        const typename IdentityOperator<BasisFunctionType, ResultType>::LocalAssemblerFactory& factory,
        const AssemblyOptions& options) const
{

    const Space<BasisFunctionType>& testSpace = this->testSpace();
    const Space<BasisFunctionType>& trialSpace = this->trialSpace();

    if (!testSpace.dofsAssigned() || !trialSpace.dofsAssigned())
        throw std::runtime_error("IdentityOperator::assembleWeakForm(): "
                                 "degrees of freedom must be assigned "
                                 "before calling assembleWeakForm()");
    if (&testSpace.grid() != &trialSpace.grid())
        throw std::runtime_error("IdentityOperator::assembleWeakForm(): "
                                 "testSpace and trialSpace must be defined over "
                                 "the same grid");

    // Prepare local assembler

    const Grid& grid = trialSpace.grid();
    std::auto_ptr<GridView> view = grid.leafView();

    // Gather geometric data
    Fiber::RawGridGeometry<CoordinateType> rawGeometry(grid.dim(), grid.dimWorld());
    view->getRawElementData(
                rawGeometry.vertices(), rawGeometry.elementCornerIndices(),
                rawGeometry.auxData());

    // Make geometry factory
    std::auto_ptr<GeometryFactory> geometryFactory =
            testSpace.grid().elementGeometryFactory();

    // Get pointers to test and trial bases of each element
    std::vector<const Fiber::Basis<BasisFunctionType>*> testBases;
    std::vector<const Fiber::Basis<BasisFunctionType>*> trialBases;
    getAllBases(testSpace, testBases);
    getAllBases(trialSpace, trialBases);

    Fiber::OpenClHandler<CoordinateType, int> openClHandler(
                options.parallelisationOptions().openClOptions());
    if (openClHandler.UseOpenCl())
        openClHandler.pushGeometry (rawGeometry.vertices(),
                                    rawGeometry.elementCornerIndices());

    // Now create the assembler
    std::auto_ptr<LocalAssembler> assembler =
            factory.makeAssemblerForIdentityOperators(
                *geometryFactory, rawGeometry,
                testBases, trialBases,
                m_expression, m_expression,
                openClHandler);

    return assembleWeakFormInternal(*assembler, options);
}

template <typename BasisFunctionType, typename ResultType>
std::auto_ptr<DiscreteLinearOperator<ResultType> >
IdentityOperator<BasisFunctionType, ResultType>::assembleWeakFormInternal(
        LocalAssembler& assembler,
        const AssemblyOptions& options) const
{
    switch (options.operatorRepresentation())
    {
    case AssemblyOptions::DENSE:
        return assembleWeakFormInDenseMode(assembler, options);
    case AssemblyOptions::ACA:
        return assembleWeakFormInSparseMode(assembler, options);
    default:
        throw std::runtime_error("IdentityOperator::assembleWeakForm(): "
                                 "invalid assembly mode");
    }
}

template <typename BasisFunctionType, typename ResultType>
std::auto_ptr<DiscreteLinearOperator<ResultType> >
IdentityOperator<BasisFunctionType, ResultType>::assembleWeakFormInDenseMode(
        typename IdentityOperator<BasisFunctionType, ResultType>::LocalAssembler& assembler,
        const AssemblyOptions& options) const
{
    const Space<BasisFunctionType>& testSpace = this->testSpace();
    const Space<BasisFunctionType>& trialSpace = this->trialSpace();

    // Fill local submatrices
    std::auto_ptr<GridView> view = testSpace.grid().leafView();
    const int elementCount = view->entityCount(0);
    std::vector<int> elementIndices(elementCount);
    for (int i = 0; i < elementCount; ++i)
        elementIndices[i] = i;
    std::vector<arma::Mat<ResultType> > localResult;
    assembler.evaluateLocalWeakForms(elementIndices, localResult);

    // Create the operator's matrix
    arma::Mat<ResultType> result(testSpace.globalDofCount(),
                                 trialSpace.globalDofCount());
    result.fill(0.);

    // Retrieve global DOFs corresponding to local DOFs on all elements
    std::vector<std::vector<GlobalDofIndex> > trialGdofs(elementCount);
    std::vector<std::vector<GlobalDofIndex> > testGdofs(elementCount);

    // Gather global DOF lists
    const Mapper& mapper = view->elementMapper();
    std::auto_ptr<EntityIterator<0> > it = view->entityIterator<0>();
    while (!it->finished())
    {
        const Entity<0>& element = it->entity();
        const int elementIndex = mapper.entityIndex(element);
        testSpace.globalDofs(element, testGdofs[elementIndex]);
        trialSpace.globalDofs(element, trialGdofs[elementIndex]);
        it->next();
    }

    // Distribute local matrices into the global matrix
    for (int e = 0; e < elementCount; ++e)
        for (int trialIndex = 0; trialIndex < trialGdofs[e].size(); ++trialIndex)
            for (int testIndex = 0; testIndex < testGdofs[e].size(); ++testIndex)
                result(testGdofs[e][testIndex], trialGdofs[e][trialIndex]) +=
                        localResult[e](testIndex, trialIndex);

    return std::auto_ptr<DiscreteLinearOperator<ResultType> >(
                new DiscreteDenseLinearOperator<ResultType>(result));
}

template <typename BasisFunctionType, typename ResultType>
std::auto_ptr<DiscreteLinearOperator<ResultType> >
IdentityOperator<BasisFunctionType, ResultType>::assembleWeakFormInSparseMode(
        typename IdentityOperator<BasisFunctionType, ResultType>::LocalAssembler& assembler,
        const AssemblyOptions& options) const
{
#ifdef WITH_TRILINOS
    if (boost::is_complex<BasisFunctionType>::value)
        throw std::runtime_error(
                "IdentityOperator::assembleWeakFormInSparseMode(): "
                "sparse-mode assembly of identity operators for "
                "complex-valued basis functions is not supported yet");

    const Space<BasisFunctionType>& testSpace = this->testSpace();
    const Space<BasisFunctionType>& trialSpace = this->trialSpace();

    // Fill local submatrices
    std::auto_ptr<GridView> view = testSpace.grid().leafView();
    const int elementCount = view->entityCount(0);
    std::vector<int> elementIndices(elementCount);
    for (int i = 0; i < elementCount; ++i)
        elementIndices[i] = i;
    std::vector<arma::Mat<ResultType> > localResult;
    assembler.evaluateLocalWeakForms(elementIndices, localResult);

    // Estimate number of entries in each row

    //    This will be useful when we begin to use MPI
    //    // Get global DOF indices for which this process is responsible
    //    const int testGlobalDofCount = testSpace.globalDofCount();
    //    Epetra_Map rowMap(testGlobalDofCount, 0 /* index-base */, comm);
    //    std::vector<int> myTestGlobalDofs(rowMap.MyGlobalElements(),
    //                                      rowMap.MyGlobalElements() +
    //                                      rowMap.NumMyElements());
    //    const int myTestGlobalDofCount = myTestGlobalDofs.size();

    const int testGlobalDofCount = testSpace.globalDofCount();
    const int trialGlobalDofCount = trialSpace.globalDofCount();
    arma::Col<int> nonzeroEntryCountEstimates(testGlobalDofCount);
    nonzeroEntryCountEstimates.fill(0);

    // Global DOF indices corresponding to local DOFs on elements
    std::vector<std::vector<GlobalDofIndex> > trialGdofs(elementCount);
    std::vector<std::vector<GlobalDofIndex> > testGdofs(elementCount);

    // Fill above lists
    const Mapper& mapper = view->elementMapper();
    std::auto_ptr<EntityIterator<0> > it = view->entityIterator<0>();
    while (!it->finished())
    {
        const Entity<0>& element = it->entity();
        const int elementIndex = mapper.entityIndex(element);
        testSpace.globalDofs(element, testGdofs[elementIndex]);
        trialSpace.globalDofs(element, trialGdofs[elementIndex]);
        it->next();
    }

    // Upper estimate for the number of global trial DOFs coupled to a given
    // global test DOF: sum of the local trial DOF counts for each element that
    // contributes to the global test DOF in question
    for (int e = 0; e < elementCount; ++e)
        for (int testLdof = 0; testLdof < testGdofs[e].size(); ++testLdof)
            nonzeroEntryCountEstimates(testGdofs[e][testLdof]) +=
                    trialGdofs[e].size();

    Epetra_SerialComm comm; // To be replaced once we begin to use MPI
    Epetra_LocalMap rowMap(testGlobalDofCount, 0 /* index_base */, comm);
    Epetra_LocalMap colMap(trialGlobalDofCount, 0 /* index_base */, comm);
    std::auto_ptr<Epetra_FECrsMatrix> result(
                new Epetra_FECrsMatrix(Copy, rowMap, colMap,
                                       nonzeroEntryCountEstimates.memptr()));

    // TODO: make each process responsible for a subset of elements
    // Find maximum number of local dofs per element
    size_t maxLdofCount = 0;
    for (int e = 0; e < elementCount; ++e)
        maxLdofCount = std::max(maxLdofCount,
                                testGdofs[e].size() * trialGdofs[e].size());

    // Initialise sparse matrix with zeros at required positions
    arma::Col<double> zeros(maxLdofCount);
    zeros.fill(0.);
    for (int e = 0; e < elementCount; ++e)
        result->InsertGlobalValues(testGdofs[e].size(), &testGdofs[e][0],
                                   trialGdofs[e].size(), &trialGdofs[e][0],
                                   zeros.memptr());
    // Add contributions from individual elements
    for (int e = 0; e < elementCount; ++e)
        epetraSumIntoGlobalValues(
                    *result, testGdofs[e], trialGdofs[e], localResult[e]);
    result->GlobalAssemble();

    // Create and return a discrete operator represented by the matrix that
    // has just been calculated
    return std::auto_ptr<DiscreteLinearOperator<ResultType> >(
                new DiscreteSparseLinearOperator<ResultType>(result));
#else // WITH_TRILINOS
    throw std::runtime_error("To enable assembly in sparse mode, recompile BEM++ "
                             "with the symbol WITH_TRILINOS defined.");
#endif
}

template <typename BasisFunctionType, typename ResultType>
std::auto_ptr<typename IdentityOperator<BasisFunctionType, ResultType>::LocalAssembler>
IdentityOperator<BasisFunctionType, ResultType>::makeAssembler(
        const LocalAssemblerFactory& assemblerFactory,
        const GeometryFactory& geometryFactory,
        const Fiber::RawGridGeometry<CoordinateType>& rawGeometry,
        const std::vector<const Fiber::Basis<BasisFunctionType>*>& testBases,
        const std::vector<const Fiber::Basis<BasisFunctionType>*>& trialBases,
        const Fiber::OpenClHandler<CoordinateType, int>& openClHandler,
        const ParallelisationOptions&,
        bool /* cacheSingularIntegrals */) const
{
    return assemblerFactory.makeAssemblerForIdentityOperators(
                geometryFactory, rawGeometry,
                testBases, trialBases,
                m_expression, m_expression,
                openClHandler);
}

FIBER_INSTANTIATE_CLASS_TEMPLATED_ON_BASIS_AND_RESULT(IdentityOperator);

} // namespace Bempp
