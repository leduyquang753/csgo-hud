#ifndef CSGO_HUD_MOVEMENT_LINEARMOVEMENTFUNCTION_H
#define CSGO_HUD_MOVEMENT_LINEARMOVEMENTFUNCTION_H

#include <vector>

#include "movement/MovementFunction.h"

namespace CsgoHud {

/*
	A movement function that consists of linear segments between the anchor points.
*/
class LinearMovementFunction final: public MovementFunction {
	private:
		const std::vector<D2D1_POINT_2F> points;
	public:
		LinearMovementFunction(const std::vector<D2D1_POINT_2F> &points);
		float getValue(float x) const override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_MOVEMENT_LINEARMOVEMENTFUNCTION_H