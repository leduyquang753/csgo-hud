#ifndef CSGO_HUD_MOVEMENT_MOVEMENTFUNCTION_H
#define CSGO_HUD_MOVEMENT_MOVEMENTFUNCTION_H

namespace CsgoHud {

/*
	A normal X-Y function to be used for animation.
*/
class MovementFunction {
	public:
		virtual ~MovementFunction() = default;
		virtual float getValue(float x) const = 0;
};

} // namespace CsgoHud

#endif // CSGO_HUD_MOVEMENT_MOVEMENTFUNCTION_H