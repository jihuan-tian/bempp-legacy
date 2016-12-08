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

#ifndef bempp_cuda_dense_global_assembler_hpp
#define bempp_cuda_dense_global_assembler_hpp

/** \file . */

#include "cuda_options.hpp"

#include "../common/common.hpp"
#include "../common/eigen_support.hpp"
#include "../common/scalar_traits.hpp"
#include "../common/shared_ptr.hpp"

#include "../fiber/_2d_array.hpp"
#include "../fiber/collection_of_kernels.hpp"

namespace Fiber {
/** \cond FORWARD_DECL */
template <typename ResultType> class LocalAssemblerForIntegralOperators;
template <typename BasisFunctionType, typename KernelType, typename ResultType,
typename GeometryFactor> class DefaultLocalAssemblerForIntegralOperatorsOnSurfaces;
template <typename BasisFunctionType> class Shapeset;
/** \endcond */
} // namespace Fiber

namespace Bempp {

/** \cond FORWARD_DECL */
template <typename ValueType> class DiscreteBoundaryOperator;
template <typename BasisFunctionType> class Space;
template <typename BasisFunctionType, typename ResultType> class Context;
/** \endcond */

/** \ingroup weak_form_assembly_internal
 * * \brief Dense-mode assembler with CUDA.
 * */
template <typename BasisFunctionType, typename ResultType>
class CudaDenseGlobalAssembler {
public:

  // TODO: How to determine the correct kernel type?
  typedef ResultType KernelType;

  typedef Fiber::Shapeset<BasisFunctionType> Shapeset;
  typedef Fiber::_2dArray<std::pair<int, Matrix<ResultType>>> Cache;

  typedef Fiber::LocalAssemblerForIntegralOperators<ResultType>
      LocalAssemblerForIntegralOperators;

  static std::unique_ptr<DiscreteBoundaryOperator<ResultType>>
  assembleDetachedWeakForm(
      const Space<BasisFunctionType> &testSpace,
      const Space<BasisFunctionType> &trialSpace,
      LocalAssemblerForIntegralOperators &assembler,
      const Context<BasisFunctionType, ResultType> &context);

private:
  template<typename CudaBasisFunctionType, typename CudaKernelType,
  typename CudaResultType>
  static void assembleDetachedWeakFormImpl(
      const Space<BasisFunctionType> &testSpace,
      const Space<BasisFunctionType> &trialSpace,
      const shared_ptr<const Fiber::CollectionOfKernels<KernelType>> &kernel,
      const Shapeset &testShapeset, const Shapeset &trialShapeset,
      const Cache &cache,
      const CudaOptions &cudaOptions,
      Matrix<ResultType> &result);
};

} // namespace Bempp

#endif
