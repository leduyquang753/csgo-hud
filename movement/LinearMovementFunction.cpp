#include "pch.h"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include "movement/MovementFunction.h"

#include "movement/LinearMovementFunction.h"

namespace CsgoHud {

// == LinearMovementFunction ==

LinearMovementFunction::LinearMovementFunction(const std::vector<D2D1_POINT_2F> &points): points(points) {
	if (points.empty()) throw std::invalid_argument("No anchor points are supplied.");
	for (std::size_t i = 1; i != points.size(); ++i) {
		if (points[i].x < points[i-1].x)
			throw std::invalid_argument("An anchor point is to the left of the previous anchor point.");
	}
}

float LinearMovementFunction::getValue(const float x) const {
	const auto firstGreater = std::upper_bound(
		points.begin(), points.end(), D2D1_POINT_2F{x, 0},
		[](const D2D1_POINT_2F &a, const D2D1_POINT_2F &b) {
			return a.x < b.x;
		}
	);
	if (firstGreater == points.begin()) {
		return points.front().y;
	} else if (firstGreater == points.end()) {
		return points.back().y;
	} else {
		const auto &a = firstGreater[-1], &b = firstGreater[0];
		return a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x);
	}
}

} // namespace CsgoHud