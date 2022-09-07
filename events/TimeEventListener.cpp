#include <cstdlib>
#include <functional>

#include "pch.h"

#include "events/EventBus.h"

#include "events/TimeEventListener.h"

namespace CsgoHud {

class EventBus;

// == TimeEventListener ==

TimeEventListener::TimeEventListener(
	EventBus *const eventBus, const std::size_t index, TimeEventListener **const slot
): eventBus(eventBus), index(index), slot(slot) {}

TimeEventListener::TimeEventListener(TimeEventListener &&other):
	eventBus(other.eventBus), index(other.index), slot(other.slot)
{
	*slot = this;
	// Make sure something happens when the moved listener is used.
	other.eventBus = nullptr;
}

TimeEventListener& TimeEventListener::operator=(TimeEventListener &&other) {
	eventBus = other.eventBus;
	index = other.index;
	slot = other.slot;
	*slot = this;
	other.eventBus = nullptr;
	return *this;
}

void TimeEventListener::unregister() {
	eventBus->unregisterTimeEventListener(*this);
}

} // namespace CsgoHud