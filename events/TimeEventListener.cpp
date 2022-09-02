#include <cstdlib>
#include <functional>

#include "pch.h"

#include "events/EventBus.h"

#include "events/TimeEventListener.h"

namespace CsgoHud {

class EventBus;

// == TimeEventListener ==

TimeEventListener::TimeEventListener(
	EventBus *const eventBus, const std::size_t index, const std::function<void(int)> &callback
): eventBus(eventBus), index(index), callback(callback) {}

void TimeEventListener::unregister() {
	eventBus->unregisterTimeEventListener(this);
}

} // namespace CsgoHud