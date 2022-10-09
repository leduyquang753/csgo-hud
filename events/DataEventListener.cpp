#include "pch.h"

#include <cstdlib>
#include <string>
#include <utility>

#include "events/EventBus.h"

#include "events/DataEventListener.h"

namespace CsgoHud {

// == DataEventListener ==

DataEventListener::DataEventListener(EventBus *const eventBus, const std::string &dataPath, const std::size_t index):
	eventBus(eventBus), dataPath(dataPath), index(index)
{
	eventBus->updateDataEventListenerSlot(dataPath, index, this);
}

DataEventListener::DataEventListener(DataEventListener &&other):
	eventBus(other.eventBus), dataPath(std::move(other.dataPath)), index(other.index)
{
	eventBus->updateDataEventListenerSlot(dataPath, index, this);
	// Make sure something happens when the move listener is used.
	other.eventBus = nullptr;
}

DataEventListener& DataEventListener::operator=(DataEventListener &&other) {
	eventBus = other.eventBus;
	dataPath = std::move(other.dataPath);
	index = other.index;
	eventBus->updateDataEventListenerSlot(dataPath, index, this);
	other.eventBus = nullptr;
	return *this;
}

DataEventListener::~DataEventListener() {
	if (index != -1) eventBus->updateDataEventListenerSlot(dataPath, index, nullptr);
}

void DataEventListener::unregister() {
	eventBus->unregisterDataEventListener(*this);
}

} // namespace CsgoHud