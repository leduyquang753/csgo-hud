#include "pch.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include "movement/MovementFunction.h"

#include "movement/CubicBezierMovementFunction.h"

namespace CsgoHud {

// == CubicBezierMovementFunction ==

CubicBezierMovementFunction::CubicBezierMovementFunction(const std::vector<D2D1_POINT_2F> &points) {
	const std::size_t curveCount = (points.size() - 1) / 3;
	if (points.size() % 3 != 1 || curveCount == 0)
		throw std::invalid_argument("The number of points must be 3n+1 where n is a positive integer.");
	anchors.reserve(curveCount + 1);
	curves.reserve(curveCount);

	anchors.push_back(points.front());
	for (auto curvePoints = points.begin() + 1; curvePoints != points.end(); curvePoints += 3) {
		if (curvePoints[2].x < anchors.back().x)
			throw std::invalid_argument("An anchor point is to the left of the previous anchor point.");
		else if (curvePoints[2].x == anchors.back().x)
			curves.emplace_back(0.5f, 0.5f, 0.5f, 0.5f); // Just a placeholder, won't be used.
		else
			curves.emplace_back(curvePoints[0].x, curvePoints[0].y, curvePoints[1].x, curvePoints[1].y);
		anchors.push_back(curvePoints[2]);
	}
}

float CubicBezierMovementFunction::getValue(const float x) const {
	const auto firstGreater = std::upper_bound(
		anchors.begin(), anchors.end(), D2D1_POINT_2F{x, 0},
		[](const D2D1_POINT_2F &a, const D2D1_POINT_2F &b) {
			return a.x < b.x;
		}
	);
	if (firstGreater == anchors.begin()) {
		return anchors.front().y;
	} else if (firstGreater == anchors.end()) {
		return anchors.back().y;
	} else {
		const std::size_t curveIndex = firstGreater - anchors.begin() - 1;
		return
			anchors[curveIndex].y
			+ (anchors[curveIndex+1].y - anchors[curveIndex].y) * curves[curveIndex](
				(x - anchors[curveIndex].x) / (anchors[curveIndex+1].x - anchors[curveIndex].x)
			);
	}
}

} // namespace CsgoHud