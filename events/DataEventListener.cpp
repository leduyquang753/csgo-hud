#include <cstdlib>
#include <functional>
#include <string>
#include <utility>

#include "pch.h"

#include "events/EventBus.h"

#include "events/DataEventListener.h"

namespace CsgoHud {

// == DataEventListener ==

DataEventListener::DataEventListener(
	EventBus *const eventBus, const std::string &dataPath, const std::size_t index, DataEventListener **const slot
): eventBus(eventBus), dataPath(dataPath), index(index), slot(slot) {}

DataEventListener::DataEventListener(DataEventListener &&other):
	eventBus(other.eventBus), dataPath(std::move(other.dataPath)), index(other.index), slot(other.slot)
{
	*slot = this;
	// Make sure something happens when the moved listener is used.
	other.eventBus = nullptr;
}

DataEventListener& DataEventListener::operator=(DataEventListener &&other) {
	eventBus = other.eventBus;
	dataPath = std::move(other.dataPath);
	index = other.index;
	slot = other.slot;
	*slot = this;
	other.eventBus = nullptr;
	return *this;
}

void DataEventListener::unregister() {
	eventBus->unregisterDataEventListener(*this);
}

} // namespace CsgoHud