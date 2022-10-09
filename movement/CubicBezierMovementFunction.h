#ifndef CSGO_HUD_MOVEMENT_CUBICBEZIERMOVEMENTFUNCTION_H
#define CSGO_HUD_MOVEMENT_CUBICBEZIERMOVEMENTFUNCTION_H

#include <vector>

#include "movement/CubicBezier.h"
#include "movement/MovementFunction.h"

namespace CsgoHud {

/*
	A movement function that consists of consecutive cubic Bézier curves.
*/
class CubicBezierMovementFunction final: public MovementFunction {
	private:
		std::vector<D2D1_POINT_2F> anchors;
		std::vector<CubicBezier> curves;
	public:
		/*
			`points` consist of first the starting point, then a number of groups of three points: the curve handle of
			the first point, the curve handle of the second point, then the second anchor point. The curve handles'
			coordinates are normalized, that is, the X coordinate from 0 to 1 is from the first anchor's X to the
			second anchor's X, the Y coordinate from 0 to 1 is from the first anchor's Y to the second anchor's Y.
			If a handle's X coordinate is outside of the range [0; 1] it will be clamped.
		*/
		CubicBezierMovementFunction(const std::vector<D2D1_POINT_2F> &points);
		float getValue(float x) const override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_MOVEMENT_CUBICBEZIERMOVEMENTFUNCTION_H