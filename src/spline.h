#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <iostream>

struct FrenetFrame {
    glm::vec3 tangent;
    glm::vec3 normal;
    glm::vec3 binormal;
};

class BSpline {
private:
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> orientationVectors;
    std::vector<float> knotVector;
    int degree;

    // Calculate the basis function N_i,p(u)
    float basisFunction(int i, int p, float u, const std::vector<float>& knots) const {
        if (p == 0) {
            return (u >= knots[i] && u < knots[i + 1]) ? 1.0f : 0.0f;
        }

        float left = 0.0f, right = 0.0f;

        if (knots[i + p] != knots[i]) {
            left = (u - knots[i]) / (knots[i + p] - knots[i]) *
                   basisFunction(i, p - 1, u, knots);
        }

        if (knots[i + p + 1] != knots[i + 1]) {
            right = (knots[i + p + 1] - u) / (knots[i + p + 1] - knots[i + 1]) *
                    basisFunction(i + 1, p - 1, u, knots);
        }

        return left + right;
    }

public:
    BSpline(const std::vector<glm::vec3>& points,
              const std::vector<glm::vec3>& orientations,
              int deg = 3)
        : controlPoints(points), orientationVectors(orientations), degree(deg) {
        generateKnotVector();
    }

    void generateKnotVector() {
        int n = controlPoints.size();
        int m = n + degree + 1;
        knotVector.resize(m);

        // Clamped knot vector
        for (int i = 0; i <= degree; ++i) {
            knotVector[i] = 0.0f;
        }

        for (int i = degree + 1; i < n; ++i) {
            knotVector[i] = float(i - degree) / float(n - degree);
        }

        for (int i = n; i < m; ++i) {
            knotVector[i] = 1.0f;
        }
    }

    // Compute approximate arc length from 0 to t
    float arcLength(float t, int samples = 100) const {
        if (t <= 0) return 0.0f;

        float length = 0.0f;
        float dt = t / float(samples);

        for (int i = 0; i < samples; ++i) {
            float t1 = float(i) * dt;
            float t2 = float(i + 1) * dt;
            glm::vec3 p1 = evaluate(t1);
            glm::vec3 p2 = evaluate(t2);
            length += glm::length(p2 - p1);
        }

        return length;
    }

    // Find parameter t that gives desired arc length
    float parameterFromArcLength(float targetLength, float totalLength) const {
        if (targetLength <= 0) return 0.0f;
        if (targetLength >= totalLength) return 1.0f;

        // Binary search for the parameter
        float low = 0.0f, high = 1.0f;

        for (int iter = 0; iter < 20; ++iter) {
            float mid = (low + high) * 0.5f;
            float currentLength = arcLength(mid);

            if (currentLength < targetLength) {
                low = mid;
            } else {
                high = mid;
            }
        }

        return (low + high) * 0.5f;
    }

    glm::vec3 evaluate(float t) const {
        // Clamp t and handle edge case
        t = std::clamp(t, 0.0f, 1.0f - 1e-6f);

        glm::vec3 point(0.0f);
        int n = controlPoints.size();

        for (int i = 0; i < n; ++i) {
            float basis = basisFunction(i, degree, t, knotVector);
            point += basis * controlPoints[i];
        }

        return point;
    }

    glm::vec3 evaluateOrientation(float t) const {
        t = std::clamp(t, 0.0f, 1.0f - 1e-6f);

        glm::vec3 orientation(0.0f);
        int n = controlPoints.size();

        for (int i = 0; i < n; ++i) {
            float basis = basisFunction(i, degree, t, knotVector);
            orientation += basis * orientationVectors[i];
        }

        // Make it normal to spline
        glm::vec3 tangent = derivative(t);
        orientation = -glm::cross(tangent, glm::cross(tangent, orientation));

        return glm::normalize(orientation);
    }

    // Evaluate derivative at parameter t
    glm::vec3 derivative(float t) const {
        const float h = 1e-5f;

        if (t <= h) {
            // Forward difference at start
            return (evaluate(t + h) - evaluate(t)) / h;
        } else if (t >= 1.0f - h) {
            // Backward difference at end
            return (evaluate(t) - evaluate(t - h)) / h;
        } else {
            // Central difference in middle
            return (evaluate(t + h) - evaluate(t - h)) / (2.0f * h);
        }
    }

    // Generate points along the curve
    std::vector<glm::vec3> generateCurve(int numPoints = 100) const {
        std::vector<glm::vec3> curve;
        curve.reserve(numPoints);

        for (int i = 0; i < numPoints; ++i) {
            float t = float(i) / float(numPoints - 1);
            curve.push_back(evaluate(t));
        }

        return curve;
    }

    // Getters and setters
    const std::vector<glm::vec3>& getControlPoints() const { return controlPoints; }
    void setControlPoint(int index, const glm::vec3& point) {
        if (index >= 0 && index < controlPoints.size()) {
            controlPoints[index] = point;
        }
    }

    int getDegree() const { return degree; }
    const std::vector<float>& getKnotVector() const { return knotVector; }
};

// Utility class for B-spline surface (tensor product)
class BSplineSurface {
private:
    std::vector<std::vector<glm::vec3>> controlNet;
    int degreeU, degreeV;
    std::vector<float> knotVectorU, knotVectorV;

    float basisFunction(int i, int p, float u, const std::vector<float>& knots) const {
        if (p == 0) {
            return (u >= knots[i] && u < knots[i + 1]) ? 1.0f : 0.0f;
        }

        float left = 0.0f, right = 0.0f;

        if (knots[i + p] != knots[i]) {
            left = (u - knots[i]) / (knots[i + p] - knots[i]) *
                   basisFunction(i, p - 1, u, knots);
        }

        if (knots[i + p + 1] != knots[i + 1]) {
            right = (knots[i + p + 1] - u) / (knots[i + p + 1] - knots[i + 1]) *
                    basisFunction(i + 1, p - 1, u, knots);
        }

        return left + right;
    }

    void generateKnotVector(std::vector<float>& knots, int numPoints, int degree) {
        int m = numPoints + degree + 1;
        knots.resize(m);

        for (int i = 0; i <= degree; ++i) {
            knots[i] = 0.0f;
        }

        for (int i = degree + 1; i < numPoints; ++i) {
            knots[i] = float(i - degree) / float(numPoints - degree);
        }

        for (int i = numPoints; i < m; ++i) {
            knots[i] = 1.0f;
        }
    }

public:
    BSplineSurface(const std::vector<std::vector<glm::vec3>>& net,
                   int degU = 3, int degV = 3)
        : controlNet(net), degreeU(degU), degreeV(degV) {
        generateKnotVector(knotVectorU, controlNet.size(), degreeU);
        generateKnotVector(knotVectorV, controlNet[0].size(), degreeV);
    }

    glm::vec3 evaluate(float u, float v) const {
        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        glm::vec3 point(0.0f);
        int numU = controlNet.size();
        int numV = controlNet[0].size();

        for (int i = 0; i < numU; ++i) {
            for (int j = 0; j < numV; ++j) {
                float basisU = basisFunction(i, degreeU, u, knotVectorU);
                float basisV = basisFunction(j, degreeV, v, knotVectorV);
                point += basisU * basisV * controlNet[i][j];
            }
        }

        return point;
    }
};
