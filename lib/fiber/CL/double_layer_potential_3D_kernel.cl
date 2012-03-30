// -*-C++-*-

/**
 * \file single_layer_potential_3D_kernel.cl
 * OpenCL implementation for single layer potential kernel evaluation
 */

/**
 * \brief Single layer potential evaluation for a single point pair
 * \param testPoint test point coordinates
 * \param trialPoint trial point coordinates
 * \param coordCount number of coordinates for each point
 * \note testPoint and trialPoint must be of size coordCount
 */
ValueType devKerneval (const ValueType *testPoint,
		       const ValueType *trialPoint,
	               const ValueType *trialNormal,
		       int coordCount)
{
    int k;
    ValueType diff, distance, denominatorSum, numeratorSum;
    denominatorSum = 0;
    numeratorSum = 0;
    for (k = 0; k < coordCount; k++) {
        diff = testPoint[k] - trialPoint[k];
	denominatorSum += diff * diff;
	numeratorSum += diff * trialNormal[k];
    }
    distance = sqrt (denominatorSum);
    return -numeratorSum / (4.0 * M_PI * distance * distance * distance);
}