#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class BSpline {
private:
    // Control points defining the shape of the spline
    std::vector<glm::vec3> controlPoints;
    // Vector corresponding to each control point, defining the orientation of the spline at that point
    std::vector<glm::vec3> orientationVectors;
    // Polynomial order of the spline (e.g. 2 for quadratic, 3 for cubic)
    int degree;
    // Knots are the points where spline segments meet
    std::vector<float> knots;
    // The arc length is the physical length of the spline in 3d space.
    // Computing it at runtime can be resource intensive, so we keep a cache of
    // the arc length at each control point and interpolate between those.
    std::vector<float> arcLengthCache;

    // Compute position of knots along the spline
    void computeKnots();

    // Create arc length cache
    void cacheArcLengths();

    // The basis function computes the influence of a control point
    // at a given position along the spline.
    float basisFunction(int i, int p, float u) const;

public:
    // Create a B-Spline given a set of points and corresponding orientations,
    // and the polynomial degree of the spline
    // TODO Take a single vector of [point, orientation] to enforce alignment?
    BSpline(const std::vector<glm::vec3>& controlPoints,
            const std::vector<glm::vec3>& orientationVectors,
            int degree = 3);

    // Getters
    const std::vector<glm::vec3>& getControlPoints() {
        return controlPoints;
    }
    const std::vector<glm::vec3>& getOrientationVectors() {
        return orientationVectors;
    }

    // Use cache to compute approximate arc length from 0 to t
    float arcLength(float t);

    // Find parameter t that gives desired arc length
    float parameterFromArcLength(float targetLength, float totalLength);

    // Evaluate the spline at parameter t (fraction from 0 to 1)
    glm::vec3 evaluate(float t) const;
    glm::vec3 evaluateOrientation(float t) const;

    // Evaluate derivative at parameter t via finite difference method
    glm::vec3 derivative(float t) const;

    // Generate evenly spaced points along the curve
    std::vector<glm::vec3> generateCurve(int numPoints = 100) const;
};

// For testing; create a spline with hardcoded points, optionally repeated to create a bigger structure
BSpline exampleSpline(bool large);
