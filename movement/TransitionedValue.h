#ifndef CSGO_HUD_MOVEMENT_TRANSITIONEDVALUE_H
#define CSGO_HUD_MOVEMENT_TRANSITIONEDVALUE_H

#include <memory>

#include "movement/MovementFunction.h"

namespace CsgoHud {

struct CommonResources;
struct TimeEventListener;

/*
	A value that can transition over time from the previous to the newly set value using a movement function.
	The movement function has 0 as the previous value and 1 as the next value in the transition.
*/
class TransitionedValue final {
	private:
		const std::unique_ptr<MovementFunction> movementFunction;
		const int transitionTime;
		int passedTime;
		float previousValue;
		float currentValue;
		TimeEventListener *const timeEventListener;

		void advanceTime(int timePassed);
	public:
		TransitionedValue(
			CommonResources &commonResources,
			std::unique_ptr<MovementFunction> &&movementFunction,
			int transitionTime, float initialValue
		);
		TransitionedValue(TransitionedValue &&source) = delete;
		~TransitionedValue();
		bool transiting() const;
		float getValue() const;
		void setValue(float value);
		void transition(float newValue);
};

} // namespace CsgoHud

#endif // CSGO_HUD_MOVEMENT_TRANSITIONEDVALUE_H