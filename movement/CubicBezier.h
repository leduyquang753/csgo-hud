#ifndef CSGO_HUD_MOVEMENT_CUBICBEZIER_H
#define CSGO_HUD_MOVEMENT_CUBICBEZIER_H

#include <array>

namespace CsgoHud {

/*
	An implementation of computing approximate values for easing functions using cubic Bézier curves.
	Based on the implementation in JavaScript by Gaëtan Renaudeau
	(https://github.com/gre/bezier-easing – MIT license).
*/
class CubicBezier final {
	private:
		static constexpr int SPLINE_TABLE_SIZE = 11;
		static const float SAMPLE_STEP_SIZE;

		const float x1, y1, x2, y2;
		std::array<float, SPLINE_TABLE_SIZE> splineTable;
	public:
		/*
			Anchor points are always (0; 0) and (1; 1), scaling of the curve can be done outside.
			If x coordinates are outside of the [0; 1] range they will be clamped.
		*/
		CubicBezier(float x1, float y1, float x2, float y2);
		float operator()(float x) const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_MOVEMENT_CUBICBEZIER_H