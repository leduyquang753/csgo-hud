#include <cstdlib>
#include <functional>
#include <string>

#include "pch.h"

#include "events/EventBus.h"

#include "events/DataEventListener.h"

namespace CsgoHud {

// == DataEventListener ==

DataEventListener::DataEventListener(
	EventBus *const eventBus, const std::string &dataPath, const std::size_t index,
	const std::function<void(const JSON&)> &callback
): eventBus(eventBus), dataPath(dataPath), index(index), callback(callback) {}

void DataEventListener::unregister() {
	eventBus->unregisterDataEventListener(this);
}

} // namespace CsgoHud