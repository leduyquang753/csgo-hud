#include "pch.h"

#include <memory>
#include <utility>

#include "events/TimeEventListener.h"
#include "movement/MovementFunction.h"
#include "resources/CommonResources.h"

#include "movement/TransitionedValue.h"

namespace CsgoHud {

// == TransitionedValue ==

TransitionedValue::TransitionedValue(
	CommonResources &commonResources,
	std::unique_ptr<MovementFunction> &&movementFunction,
	const int transitionTime, const float initialValue
):
	transitionTime(transitionTime), passedTime(transitionTime), currentValue(initialValue),
	timeEventListener(commonResources.eventBus.listenToTimeEvent([this](const int timePassed) {
		advanceTime(timePassed);
	}))
{
	movementFunctions.emplace_back(std::move(movementFunction));
}

TransitionedValue::TransitionedValue(
	CommonResources &commonResources,
	std::vector<std::unique_ptr<MovementFunction>> &&movementFunctions,
	const int transitionTime, const float initialValue
):
	movementFunctions(std::move(movementFunctions)),
	transitionTime(transitionTime), passedTime(transitionTime), currentValue(initialValue),
	timeEventListener(commonResources.eventBus.listenToTimeEvent([this](const int timePassed) {
		advanceTime(timePassed);
	}))
{}

TransitionedValue::~TransitionedValue() {
	timeEventListener.unregister();
}

void TransitionedValue::advanceTime(const int timePassed) {
	if (passedTime >= transitionTime) return;
	passedTime += timePassed;
}

bool TransitionedValue::transiting() const {
	return passedTime < transitionTime;
}

float TransitionedValue::getValue() const {
	return passedTime >= transitionTime
		? currentValue
		: previousValue + (
			(currentValue - previousValue)
			* movementFunctions[currentFunctionIndex]->getValue(static_cast<float>(passedTime))
		);
}

void TransitionedValue::setValue(const float value) {
	currentValue = value;
	passedTime = transitionTime;
}

void TransitionedValue::transition(const float newValue, const int functionIndex) {
	previousValue = getValue();
	if (previousValue == newValue) {
		passedTime = transitionTime;
	} else {
		currentFunctionIndex = functionIndex;
		currentValue = newValue;
		passedTime = 0;
	}
}

} // namespace CsgoHud