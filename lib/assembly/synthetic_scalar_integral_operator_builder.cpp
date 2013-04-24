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

#include "synthetic_scalar_integral_operator_builder.hpp"

#include "general_elementary_local_operator_imp.hpp"
#include "synthetic_integral_operator.hpp"

#include "../fiber/explicit_instantiation.hpp"

#include "../fiber/scalar_function_value_functor.hpp"
#include "../fiber/simple_test_trial_integrand_functor.hpp"
#include "../fiber/simple_test_scalar_kernel_trial_integrand_functor.hpp"

#include "../common/boost_make_shared_fwd.hpp"

namespace Bempp
{

template <typename BasisFunctionType, typename ResultType>
BoundaryOperator<BasisFunctionType, ResultType>
makeSyntheticScalarIntegralOperator(
        const BoundaryOperator<BasisFunctionType, ResultType>& internalOp,
        const shared_ptr<const Space<BasisFunctionType> >& domain,
        const shared_ptr<const Space<BasisFunctionType> >& range,
        const shared_ptr<const Space<BasisFunctionType> >& dualToRange,
        const shared_ptr<const Space<BasisFunctionType> >& internalDomain,
        const shared_ptr<const Space<BasisFunctionType> >& internalDualToRange,
        const std::string& label, int symmetry)
{
    // Note: we don't really need to care about ranges and duals to domains of
    // the internal operator. The only range space that matters is that of the
    // leftmost operator in the product.

    typedef typename ScalarTraits<BasisFunctionType>::RealType CoordinateType;

    typedef Fiber::ScalarFunctionValueFunctor<CoordinateType> ValueFunctor;
    typedef Fiber::SimpleTestTrialIntegrandFunctor<
            BasisFunctionType, ResultType> IntegrandFunctor;

    typedef GeneralElementaryLocalOperator<BasisFunctionType, ResultType> LocalOp;

    if (!internalOp.isInitialized())
        throw std::invalid_argument("makeSyntheticScalarBoundaryOperator(): "
                                    "interalOp must be initialized");
    if (!internalDomain || !internalDualToRange)
        throw std::invalid_argument("makeSyntheticScalarBoundaryOperator(): "
                                    "pointers to spaces must not be null");

    shared_ptr<const Context<BasisFunctionType, ResultType> > context =
            internalOp.context();

    if (symmetry & (SYMMETRIC | HERMITIAN) &&
            (internalDomain != internalDualToRange || domain == dualToRange))
        throw std::invalid_argument("makeSyntheticScalarBoundaryOperator(): "
                                    "symmetry incompatible with spaces");

    std::vector<BoundaryOperator<BasisFunctionType, ResultType> > testLocalOps;
    std::vector<BoundaryOperator<BasisFunctionType, ResultType> > trialLocalOps;
    if (dualToRange != internalDualToRange) {
        testLocalOps.resize(1);
        testLocalOps[0] = BoundaryOperator<BasisFunctionType, ResultType>(
                        context, boost::make_shared<LocalOp>(
                            internalDualToRange, range, dualToRange,
                            "(" + label + ")_test_value", NO_SYMMETRY,
                            ValueFunctor(), ValueFunctor(), IntegrandFunctor()));
    }
    if (domain != internalDomain && !(symmetry & (SYMMETRIC | HERMITIAN))) {
        trialLocalOps.resize(1);
        trialLocalOps[0] = BoundaryOperator<BasisFunctionType, ResultType>(
                    context, boost::make_shared<LocalOp>(
                        domain, internalDomain /* or whatever */, internalDomain,
                        ("(" + label + ")_trial_value"), NO_SYMMETRY,
                        ValueFunctor(), ValueFunctor(), IntegrandFunctor()));
    }

    typedef SyntheticIntegralOperator<BasisFunctionType, ResultType> SyntheticOp;

    if (testLocalOps.empty() && trialLocalOps.empty())
        return internalOp;
    else
        return BoundaryOperator<BasisFunctionType, ResultType>(
                    context, boost::make_shared<SyntheticOp>(
                        testLocalOps, internalOp, trialLocalOps, label,
                        symmetry));
}

#define INSTANTIATE_FUNCTION(BASIS, RESULT) \
    template BoundaryOperator<BASIS, RESULT> \
    makeSyntheticScalarIntegralOperator( \
        const BoundaryOperator<BASIS, RESULT>&, \
        const shared_ptr<const Space<BASIS> >&, \
        const shared_ptr<const Space<BASIS> >&, \
        const shared_ptr<const Space<BASIS> >&, \
        const shared_ptr<const Space<BASIS> >&, \
        const shared_ptr<const Space<BASIS> >&, \
        const std::string&, int)
FIBER_ITERATE_OVER_BASIS_AND_RESULT_TYPES(INSTANTIATE_FUNCTION);

} // namespace Bempp
