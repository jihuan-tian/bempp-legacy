%{
#include "assembly/helmholtz_3d_boundary_operator_base.hpp"
#include "assembly/helmholtz_3d_single_layer_boundary_operator.hpp"
#include "assembly/helmholtz_3d_double_layer_boundary_operator.hpp"
#include "assembly/helmholtz_3d_adjoint_double_layer_boundary_operator.hpp"
#include "assembly/helmholtz_3d_hypersingular_boundary_operator.hpp"
%}

// Redeclared because SWIG doesn't parse correctly ...::KernelType.
// So we replace it with the explicit
// typename ScalarTraits<BasisFunctionType>::ComplexType

#define shared_ptr boost::shared_ptr
namespace Bempp
{

template <typename BasisFunctionType>
BoundaryOperator<BasisFunctionType,
typename ScalarTraits<BasisFunctionType>::ComplexType>
helmholtz3dSingleLayerBoundaryOperator(
        const shared_ptr<const Context<BasisFunctionType,
        typename ScalarTraits<BasisFunctionType>::ComplexType> >& context,
        const shared_ptr<const Space<BasisFunctionType> >& domain,
        const shared_ptr<const Space<BasisFunctionType> >& range,
        const shared_ptr<const Space<BasisFunctionType> >& dualToRange,
        typename ScalarTraits<BasisFunctionType>::ComplexType waveNumber,
        const std::string& label = "",
        int symmetry = NO_SYMMETRY);
template <typename BasisFunctionType>
BoundaryOperator<BasisFunctionType,
typename ScalarTraits<BasisFunctionType>::ComplexType>
helmholtz3dDoubleLayerBoundaryOperator(
        const shared_ptr<const Context<BasisFunctionType,
        typename ScalarTraits<BasisFunctionType>::ComplexType> >& context,
        const shared_ptr<const Space<BasisFunctionType> >& domain,
        const shared_ptr<const Space<BasisFunctionType> >& range,
        const shared_ptr<const Space<BasisFunctionType> >& dualToRange,
        typename ScalarTraits<BasisFunctionType>::ComplexType waveNumber,
        const std::string& label = "",
        int symmetry = NO_SYMMETRY);
template <typename BasisFunctionType>
BoundaryOperator<BasisFunctionType,
typename ScalarTraits<BasisFunctionType>::ComplexType>
helmholtz3dAdjointDoubleLayerBoundaryOperator(
        const shared_ptr<const Context<BasisFunctionType,
        typename ScalarTraits<BasisFunctionType>::ComplexType> >& context,
        const shared_ptr<const Space<BasisFunctionType> >& domain,
        const shared_ptr<const Space<BasisFunctionType> >& range,
        const shared_ptr<const Space<BasisFunctionType> >& dualToRange,
        typename ScalarTraits<BasisFunctionType>::ComplexType waveNumber,
        const std::string& label = "",
        int symmetry = NO_SYMMETRY);
template <typename BasisFunctionType>
BoundaryOperator<BasisFunctionType,
typename ScalarTraits<BasisFunctionType>::ComplexType>
helmholtz3dHypersingularBoundaryOperator(
        const shared_ptr<const Context<BasisFunctionType,
        typename ScalarTraits<BasisFunctionType>::ComplexType> >& context,
        const shared_ptr<const Space<BasisFunctionType> >& domain,
        const shared_ptr<const Space<BasisFunctionType> >& range,
        const shared_ptr<const Space<BasisFunctionType> >& dualToRange,
        typename ScalarTraits<BasisFunctionType>::ComplexType waveNumber,
        const std::string& label = "",
        int symmetry = NO_SYMMETRY);

%feature("compactdefaultargs") helmholtz3dSingleLayerBoundaryOperator;

BEMPP_INSTANTIATE_SYMBOL_TEMPLATED_ON_BASIS(
    helmholtz3dSingleLayerBoundaryOperator);
BEMPP_INSTANTIATE_SYMBOL_TEMPLATED_ON_BASIS(
    helmholtz3dDoubleLayerBoundaryOperator);
BEMPP_INSTANTIATE_SYMBOL_TEMPLATED_ON_BASIS(
    helmholtz3dAdjointDoubleLayerBoundaryOperator);
BEMPP_INSTANTIATE_SYMBOL_TEMPLATED_ON_BASIS(
    helmholtz3dHypersingularBoundaryOperator);

} // namespace Bempp
#undef shared_ptr
