#include "pch.h"

#include <cstdlib>

#include "events/EventBus.h"

#include "events/TimeEventListener.h"

namespace CsgoHud {

class EventBus;

// == TimeEventListener ==

TimeEventListener::TimeEventListener(EventBus *const eventBus, const std::size_t index):
	eventBus(eventBus), index(index)
{
	eventBus->updateTimeEventListenerSlot(index, this);
}

TimeEventListener::TimeEventListener(TimeEventListener &&other): eventBus(other.eventBus), index(other.index) {
	eventBus->updateTimeEventListenerSlot(index, this);
	// Make sure something happens when the move listener is used.
	other.eventBus = nullptr;
}

TimeEventListener& TimeEventListener::operator=(TimeEventListener &&other) {
	eventBus = other.eventBus;
	index = other.index;
	eventBus->updateTimeEventListenerSlot(index, this);
	other.eventBus = nullptr;
	return *this;
}

TimeEventListener::~TimeEventListener() {
	if (index != -1) eventBus->updateTimeEventListenerSlot(index, nullptr);
}

void TimeEventListener::unregister() {
	eventBus->unregisterTimeEventListener(*this);
}

} // namespace CsgoHud