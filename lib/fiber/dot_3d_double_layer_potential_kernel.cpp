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

#include "dot_3d_double_layer_potential_kernel.hpp"

#include "explicit_instantiation.hpp"
#include "geometrical_data.hpp"
//#include "CL/dot_3d_double_layer_potential_kernel.cl.str"

#include <armadillo>
#include <cassert>
#include <cmath>

namespace Fiber
{

// Double potential: derivative wrt. trial normal

template <typename ValueType>
void Dot3dDoubleLayerPotentialKernel<ValueType>::addGeometricalDependencies(
        int& testGeomDeps, int& trialGeomDeps) const
{
    testGeomDeps |= GLOBALS;
    trialGeomDeps |= GLOBALS | NORMALS;
}

template <typename ValueType>
inline ValueType Dot3dDoubleLayerPotentialKernel<ValueType>::evaluateAtPointPair(
        const arma::Col<CoordinateType>& testPoint,
        const arma::Col<CoordinateType>& trialPoint,
        const arma::Col<CoordinateType>& trialNormal) const
{
    const int coordCount = testPoint.n_rows;

    CoordinateType numeratorSum = 0., denominatorSum = 0.;
    for (int coordIndex = 0; coordIndex < coordCount; ++coordIndex)
    {
        CoordinateType diff = trialPoint(coordIndex) - testPoint(coordIndex);
        denominatorSum += diff * diff;
        numeratorSum += diff * trialNormal(coordIndex);
    }
    CoordinateType distance = sqrt(denominatorSum);
    return -numeratorSum / (static_cast<ValueType>(4.0 * M_PI) * denominatorSum) *
      (m_waveNumber + static_cast<ValueType>(1.0) / distance) * exp (-m_waveNumber * distance);
}

template <typename ValueType>
void Dot3dDoubleLayerPotentialKernel<ValueType>::evaluateAtPointPairs(
        const GeometricalData<CoordinateType>& testGeomData,
        const GeometricalData<CoordinateType>& trialGeomData,
        arma::Cube<ValueType>& result) const
{
    const arma::Mat<CoordinateType>& testPoints = testGeomData.globals;
    const arma::Mat<CoordinateType>& trialPoints = trialGeomData.globals;
    const arma::Mat<CoordinateType>& trialNormals = trialGeomData.normals;

#ifndef NDEBUG
    const int worldDim = worldDimension();
    if (testPoints.n_rows != worldDim || trialPoints.n_rows != worldDim)
        throw std::invalid_argument("Dot3dDoubleLayerPotentialKernel::evaluateAtPointPairs(): "
                                    "3D coordinates required");
    if (testPoints.n_cols != trialPoints.n_cols)
        throw std::invalid_argument("Dot3dDoubleLayerPotentialKernel::evaluateAtPointPairs(): "
                                    "number of test and trial points must be equal");
    assert(trialNormals.n_rows == worldDim);
    assert(trialNormals.n_cols == trialPoints.n_cols);
#endif

    const int pointCount = testPoints.n_cols;
    result.set_size(1, 1, pointCount);
    for (int i = 0; i < pointCount; ++i)
        result(0, 0, i) = evaluateAtPointPair(
                    testPoints.unsafe_col(i), trialPoints.unsafe_col(i),
                    trialNormals.unsafe_col(i));
}

template <typename ValueType>
void Dot3dDoubleLayerPotentialKernel<ValueType>::evaluateOnGrid(
        const GeometricalData<CoordinateType>& testGeomData,
        const GeometricalData<CoordinateType>& trialGeomData,
        Array4d<ValueType>& result) const
{
    const arma::Mat<CoordinateType>& testPoints = testGeomData.globals;
    const arma::Mat<CoordinateType>& trialPoints = trialGeomData.globals;
    const arma::Mat<CoordinateType>& trialNormals = trialGeomData.normals;

#ifndef NDEBUG
    const int worldDim = worldDimension();
    if (testPoints.n_rows != worldDim || trialPoints.n_rows != worldDim)
        throw std::invalid_argument("Dot3dDoubleLayerPotentialKernel::evaluate(): "
                                    "3D coordinates required");
    assert(trialNormals.n_rows == worldDim);
    assert(trialNormals.n_cols == trialPoints.n_cols);
#endif

    const int testPointCount = testPoints.n_cols;
    const int trialPointCount = trialPoints.n_cols;
    result.set_size(1, testPointCount, 1, trialPointCount);
    for (int trialIndex = 0; trialIndex < trialPointCount; ++trialIndex)
        for (int testIndex = 0; testIndex < testPointCount; ++testIndex)
            result(0, testIndex, 0, trialIndex) = evaluateAtPointPair(
                        testPoints.unsafe_col(testIndex),
                        trialPoints.unsafe_col(trialIndex),
                        trialNormals.unsafe_col(trialIndex));
}

template<typename ValueType>
std::pair<const char*,int> Dot3dDoubleLayerPotentialKernel<ValueType>::evaluateClCode () const
{
    return std::make_pair ("", 0);  // TODO

  //    return std::make_pair(dot_3d_double_layer_potential_kernel_cl,
  //			  dot_3d_double_layer_potential_kernel_cl_len);
}

FIBER_INSTANTIATE_CLASS_TEMPLATED_ON_KERNEL(Dot3dDoubleLayerPotentialKernel);

} // namespace Fiber
