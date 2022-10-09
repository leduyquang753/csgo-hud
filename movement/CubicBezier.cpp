#include "pch.h"

#include <algorithm>
#include <cmath>

#include "movement/CubicBezier.h"

namespace CsgoHud {

// Mathy functions.

// Compute coefficients of the curve.
static float A(const float a1, const float a2) {
	return 1 + 3*a1 - 3*a2;
}

static float B(const float a1, const float a2) {
	return -6*a1 + 3*a2;
}

static float C(const float a1) {
	return 3*a1;
}

// Compute a coordinate according to `t`. `a1` and `a2` are the corresponding coordinate of the handles.
static float getBezierCoordinate(const float t, const float a1, const float a2) {
	return ((A(a1, a2) * t + B(a1, a2)) * t + C(a1)) * t;
}

/*
	Compute the derivative along an axis according to `t`.
	`a1` and `a2` are the corresponding coordinate of the handles.
*/
static float getBezierDerivative(const float t, const float a1, const float a2) {
	return 3*A(a1, a2)*t*t + 2*B(a1, a2)*t + C(a1);
}

/*
	Approximately find `t` according to the `x` coordinate using binary subdivision.
	`a` and `b` are the start and end of the search interval.
*/
static float binarySubdivide(const float x, float a, float b, const float x1, const float x2) {
	static const int MAXIMUM_ITERATIONS = 10;
	static const float SUBDIVISION_PRECISION = 1e-7f;
	float currentT;
	for (int iteration = 0; iteration != MAXIMUM_ITERATIONS; ++iteration) {
		currentT = (a+b) / 2;
		const float currentX = getBezierCoordinate(currentT, x1, x2) - x;
		if (std::abs(currentX) <= SUBDIVISION_PRECISION) break;
		(currentX > 0 ? b : a) = currentT;
	}
	return currentT;
}

/*
	Approximately find `t` according to the `x` coordinate using Newton–Raphson method.
	An initial `t` is given for the start of the search.
*/
static float newtonRaphson(const float x, float t, const float x1, const float x2) {
	static const int NEWTON_RAPHSON_ITERATIONS = 4;
	for (int iteration = 0; iteration != NEWTON_RAPHSON_ITERATIONS; ++iteration) {
		const float slope = getBezierDerivative(t, x1, x2);
		if (slope == 0) break;
		t -= (getBezierCoordinate(t, x1, x2) - x) / slope;
	}
	return t;
}

// == CubicBezier ==

const float CubicBezier::SAMPLE_STEP_SIZE = 1.f / (SPLINE_TABLE_SIZE - 1);

CubicBezier::CubicBezier(const float x1, const float y1, const float x2, const float y2):
	x1(std::clamp(x1, 0.f, 1.f)), y1(y1), x2(std::clamp(x2, 0.f, 1.f)), y2(y2)
{
	for (int i = 0; i != SPLINE_TABLE_SIZE; ++i) {
		splineTable[i] = getBezierCoordinate(SAMPLE_STEP_SIZE * i, x1, x2);
	}
}

float CubicBezier::operator()(const float x) const {
	static const float NEWTON_RAPHSON_MINIMUM_SLOPE = 0.001f;
	
	// "Guarantee the extremes are right."
	if (x == 0 || x == 1) return x;

	float intervalStart = 0;
	int currentSample = 1;
	static const int LAST_SAMPLE = SPLINE_TABLE_SIZE - 1;
	while (currentSample != LAST_SAMPLE && splineTable[currentSample] <= x) {
		intervalStart += SAMPLE_STEP_SIZE;
		++currentSample;
	}
	--currentSample;

	// Interpolate to provide an initial guess for `t`.
	const float t
		= intervalStart
	 	+ (x - splineTable[currentSample])
	 		/ (splineTable[currentSample+1] - splineTable[currentSample])
			* SAMPLE_STEP_SIZE;
	
	const float initialSlope = getBezierDerivative(t, x1, x2);
	return getBezierCoordinate(
		initialSlope >= NEWTON_RAPHSON_MINIMUM_SLOPE ? newtonRaphson(x, t, x1, x2)
		: initialSlope == 0 ? t
		: binarySubdivide(x, intervalStart, intervalStart + SAMPLE_STEP_SIZE, x1, x2),
		y1, y2
	);
}

} // namespace CsgoHud