#include "spline.h"

void BSpline::computeKnots() {
    int n = controlPoints.size();
    int m = n + degree + 1;
    knots.resize(m);
    for (int i = 0; i <= degree; ++i) {
        knots[i] = 0.0f;
    }
    for (int i = degree + 1; i < n; ++i) {
        knots[i] = float(i - degree) / float(n - degree);
    }
    for (int i = n; i < m; ++i) {
        knots[i] = 1.0f;
    }
}

void BSpline::cacheArcLengths() {
    // TODO Increase resolution?
    const int CACHE_RESOLUTION = 1; // Per segment
    int nSamples = CACHE_RESOLUTION * controlPoints.size();

    arcLengthCache.clear();
    arcLengthCache.reserve(nSamples + 1);

    arcLengthCache.push_back(0.0f);
    float dt = 1.0f / float(nSamples);
    for (int i = 0; i < nSamples; ++i) {
        float t1 = float(i) * dt;
        float t2 = float(i + 1) * dt;
        glm::vec3 p1 = evaluate(t1);
        glm::vec3 p2 = evaluate(t2);
        float segmentLength = glm::length(p2 - p1);
        arcLengthCache.push_back(arcLengthCache.back() + segmentLength);
    }
}

BSpline::BSpline(const std::vector<glm::vec3>& controlPoints,
        const std::vector<glm::vec3>& orientationVectors,
        int degree)
    : controlPoints(controlPoints), orientationVectors(orientationVectors), degree(degree) {
    assert(orientationVectors.size() == controlPoints.size());
    // NOTE Computation of knots and arc lengths can be done here because Spline is immutable.
    //      If Spline is allowed to be modified after creation, they must be recomputed.
    computeKnots();
    cacheArcLengths();
}

float BSpline::basisFunction(int i, int p, float u) const {
    if (p == 0) {
        return (u >= knots[i] && u < knots[i + 1]) ? 1.0f : 0.0f;
    }

    float left = 0.0f, right = 0.0f;
    if (knots[i + p] != knots[i]) {
        left = (u - knots[i]) / (knots[i + p] - knots[i]) *
            basisFunction(i, p - 1, u);
    }
    if (knots[i + p + 1] != knots[i + 1]) {
        right = (knots[i + p + 1] - u) / (knots[i + p + 1] - knots[i + 1]) *
            basisFunction(i + 1, p - 1, u);
    }

    return left + right;
}

float BSpline::arcLength(float t) {
    assert(arcLengthCache.size() > 0);

    // Bounds
    if (t <= 0) return 0.0f;
    if (t >= 1.0f) return arcLengthCache.back();

    // Find the segment containing t
    float index = t * float(arcLengthCache.size() - 1);
    int i0 = int(index);
    int i1 = std::min(i0 + 1, int(arcLengthCache.size() - 1));
    float frac = index - float(i0);

    // Linear interpolation between values
    return arcLengthCache[i0] * (1.0f - frac) + arcLengthCache[i1] * frac;
}

float BSpline::parameterFromArcLength(float targetLength, float totalLength) {
    assert(arcLengthCache.size() > 0);

    // Bounds
    if (targetLength <= 0) return 0.0f;
    if (targetLength >= arcLengthCache.back()) return 1.0f;

    // Binary search through the cached arc lengths
    auto it = std::lower_bound(arcLengthCache.begin(), arcLengthCache.end(), targetLength);
    int index = std::distance(arcLengthCache.begin(), it);
    if (index == 0) return 0.0f;

    // Linear interpolation between the two nearest cached values
    float lengthBefore = arcLengthCache[index - 1];
    float lengthAfter = arcLengthCache[index];
    float frac = (targetLength - lengthBefore) / (lengthAfter - lengthBefore);

    float tBefore = float(index - 1) / float(arcLengthCache.size() - 1);
    float tAfter = float(index) / float(arcLengthCache.size() - 1);

    return tBefore + frac * (tAfter - tBefore);
}

glm::vec3 BSpline::evaluate(float t) const {
    // Bounds
    if (t <= 0.0f) return controlPoints[0];
    if (t >= 1.0f) return controlPoints.back();

    // Add up control points weighed by their influences
    glm::vec3 point(0.0f);
    for (int i = 0; i < controlPoints.size(); ++i) {
        float basis = basisFunction(i, degree, t);
        point += basis * controlPoints[i];
    }

    return point;
}

glm::vec3 BSpline::evaluateOrientation(float t) const {
    // Bounds
    if (t <= 0.0f) return orientationVectors[0];
    if (t >= 1.0f) return orientationVectors.back();

    glm::vec3 orientation(0.0f);
    for (int i = 0; i < controlPoints.size(); ++i) {
        float basis = basisFunction(i, degree, t);
        orientation += basis * orientationVectors[i];
    }

    // Make it normal to spline
    glm::vec3 tangent = derivative(t);
    orientation = -glm::cross(tangent, glm::cross(tangent, orientation));

    return glm::normalize(orientation);
}

glm::vec3 BSpline::derivative(float t) const {
    const float e = 1e-5f;
    if (t <= e) {
        // Forward difference at start
        return (evaluate(t + e) - evaluate(t)) / e;
    } else if (t >= 1.0f - e) {
        // Backward difference at end
        return (evaluate(t) - evaluate(t - e)) / e;
    } else {
        // Central difference in middle
        return (evaluate(t + e) - evaluate(t - e)) / (2.0f * e);
    }
}

std::vector<glm::vec3> BSpline::generateCurve(int numPoints) const {
    std::vector<glm::vec3> curve;
    curve.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i) {
        float t = float(i) / float(numPoints - 1);
        curve.push_back(evaluate(t));
    }
    return curve;
}



BSpline exampleSpline(bool large) {
    std::vector<glm::vec3> controlPoints = {
        glm::vec3( 4.047, -12.839,  16.901),
        glm::vec3( 5.757, -11.521,  13.850),
        glm::vec3( 9.465, -10.668,  13.672),
        glm::vec3(11.204,  -8.935,  10.767),
        glm::vec3(14.831,  -9.075,   9.617),
        glm::vec3(15.122,  -5.348,   8.865),
        glm::vec3(13.127,  -2.024,   8.869),
        glm::vec3(12.305,  -2.325,   5.169),
        glm::vec3(10.923,  -5.730,   5.714),
        glm::vec3( 8.653,  -4.582,   8.562),
        glm::vec3( 7.427,  -1.567,   6.596),
        glm::vec3( 6.631,  -3.764,   3.624),
        glm::vec3( 4.949,  -6.246,   6.024),
        glm::vec3( 2.680,  -3.622,   7.319),
        glm::vec3( 1.661,  -2.534,   3.797),
        glm::vec3( 1.047,  -6.177,   2.834),
        glm::vec3(-1.295,  -6.482,   5.820),
        glm::vec3(-3.420,  -3.432,   4.910),
        glm::vec3(-5.804,  -5.385,   2.594),
    };
    std::vector<glm::vec3> orientationVectors = {
        glm::normalize(glm::vec3( 5.379, -13.834,  15.145) - glm::vec3( 4.047, -12.839,  16.901)),
        glm::normalize(glm::vec3( 7.152,  -9.941,  15.050) - glm::vec3( 5.757, -11.521,  13.850)),
        glm::normalize(glm::vec3( 9.868, -11.242,  11.386) - glm::vec3( 9.465, -10.668,  13.672)),
        glm::normalize(glm::vec3(13.302,  -8.278,  11.761) - glm::vec3(11.204,  -8.935,  10.767)),
        glm::normalize(glm::vec3(16.635,  -7.542,   9.793) - glm::vec3(14.831,  -9.075,   9.617)),
        glm::normalize(glm::vec3(12.760,  -4.866,   8.857) - glm::vec3(15.122,  -5.348,   8.865)),
        glm::normalize(glm::vec3(11.077,  -2.142,   7.663) - glm::vec3(13.127,  -2.024,   8.869)),
        glm::normalize(glm::vec3(10.150,  -3.168,   4.873) - glm::vec3(12.305,  -2.325,   5.169)),
        glm::normalize(glm::vec3( 8.570,  -5.895,   6.136) - glm::vec3(10.923,  -5.730,   5.714)),
        glm::normalize(glm::vec3( 6.496,  -3.756,   7.879) - glm::vec3( 8.653,  -4.582,   8.562)),
        glm::normalize(glm::vec3( 5.448,  -2.001,   5.264) - glm::vec3( 7.427,  -1.567,   6.596)),
        glm::normalize(glm::vec3( 4.524,  -4.862,   3.627) - glm::vec3( 6.631,  -3.764,   3.624)),
        glm::normalize(glm::vec3( 2.554,  -6.105,   6.047) - glm::vec3( 4.949,  -6.246,   6.024)),
        glm::normalize(glm::vec3( 0.594,  -3.147,   6.222) - glm::vec3( 2.680,  -3.622,   7.319)),
        glm::normalize(glm::vec3(-0.243,  -3.743,   2.943) - glm::vec3( 1.661,  -2.534,   3.797)),
        glm::normalize(glm::vec3(-1.190,  -7.047,   3.135) - glm::vec3( 1.047,  -6.177,   2.834)),
        glm::normalize(glm::vec3(-3.614,  -5.945,   5.977) - glm::vec3(-1.295,  -6.482,   5.820)),
        glm::normalize(glm::vec3(-5.776,  -3.791,   4.790) - glm::vec3(-3.420,  -3.432,   4.910)),
        glm::normalize(glm::vec3(-7.369,  -7.123,   2.981) - glm::vec3(-5.804,  -5.385,   2.54)),
    };

    if (large) {
        const int skipFirst = 8;
        const int N_Z = 4;
        const int N_Y = 3;
        int initialN = controlPoints.size();
        for (int x = 1; x < N_Z; x++) {
            for (int i = 0; i < initialN - skipFirst; i++) {
                glm::vec3 offset = glm::vec3(0.0, 0.0, -4.0 * x);
                int index = x % 2 == 0 ? skipFirst + i : initialN - i - 1;
                controlPoints.push_back(controlPoints[index] + offset);
                orientationVectors.push_back(orientationVectors[index]);
            }
        }
        initialN = controlPoints.size();
        for (int x = 1; x < N_Y; x++) {
            for (int i = 0; i < initialN - skipFirst; i++) {
                glm::vec3 offset = glm::vec3(0.0, -4.0 * x, 0.0);
                int index = x % 2 == 0 ? skipFirst + i : initialN - i - 1;
                controlPoints.push_back(controlPoints[index] + offset);
                orientationVectors.push_back(orientationVectors[index]);
            }
        }
    }

    // Center points
    glm::vec3 center;
    for (auto p : controlPoints) {
        center += p;
    }
    center /= controlPoints.size();
    for (auto &p : controlPoints) {
        p -= center;
        // Manual adjustment
        //p.x -= 2.0f;
        //p.y -= 2.0f;
        //p.z += 2.0f;
    }

    // Generate orientation vectors
    if (large) {
        orientationVectors.clear();
        for (int i = 1; i < controlPoints.size() - 1; i++) {
            glm::vec3 a = controlPoints[i - 1];
            glm::vec3 b = controlPoints[i];
            glm::vec3 c = controlPoints[i + 1];
            //glm::vec3 n = glm::normalize((b - c) + (b - a));
            glm::vec3 n = glm::normalize(glm::cross(b - c, b - a));
            orientationVectors.push_back(n);
        }
        orientationVectors.insert(orientationVectors.begin(), orientationVectors[0]);
        orientationVectors.push_back(orientationVectors.back());
    }

    return BSpline(controlPoints, orientationVectors, 3); // TODO degree 0
}
