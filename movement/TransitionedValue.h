#ifndef CSGO_HUD_MOVEMENT_TRANSITIONEDVALUE_H
#define CSGO_HUD_MOVEMENT_TRANSITIONEDVALUE_H

#include <memory>
#include <vector>

#include "events/TimeEventListener.h"
#include "movement/MovementFunction.h"

namespace CsgoHud {

struct CommonResources;

/*
	A value that can transition over time from the previous to the newly set value using a movement function.
	The movement function has 0 as the previous value and 1 as the next value in the transition.
*/
class TransitionedValue final {
	private:
		const std::vector<std::unique_ptr<MovementFunction>> movementFunctions;
		const int transitionTime;
		int passedTime;
		int currentFunctionIndex;
		float previousValue;
		float currentValue;
		TimeEventListener timeEventListener;

		void advanceTime(int timePassed);
	public:
		TransitionedValue(
			CommonResources &commonResources,
			std::unique_ptr<MovementFunction> &&movementFunction,
			int transitionTime, float initialValue
		);
		TransitionedValue(
			CommonResources &commonResources,
			std::vector<std::unique_ptr<MovementFunction>> &&movementFunctions,
			int transitionTime, float initialValue
		);
		TransitionedValue(TransitionedValue &&source) = delete;
		~TransitionedValue();
		bool transiting() const;
		float getValue() const;
		void setValue(float value);
		void transition(float newValue, int functionIndex = 0);
};

} // namespace CsgoHud

#endif // CSGO_HUD_MOVEMENT_TRANSITIONEDVALUE_H