#include <memory>
#include <functional>
#include <utility>

#include "pch.h"

#include "events/TimeEventListener.h"
#include "events/DataEventListener.h"

#include "events/EventBus.h"

namespace CsgoHud {

// == EventBus ==

TimeEventListener* EventBus::listenToTimeEvent(const std::function<void(int)> &callback) {
	return timeEventListeners.emplace_back(
		new TimeEventListener(this, timeEventListeners.size(), callback)
	).get();
}

DataEventListener* EventBus::listenToDataEvent(
	const std::string &dataPath, const std::function<void(const JSON&)> &callback
) {
	auto &listenerList = dataEventListenerMap[dataPath];
	return listenerList.emplace_back(
		new DataEventListener(this, dataPath, listenerList.size(), callback)
	).get();
}

void EventBus::unregisterTimeEventListener(TimeEventListener *const listener) {
	if (listener->index == -1) return;
	auto &listenerAtSlot = timeEventListeners[listener->index];
	listenerAtSlot = std::move(timeEventListeners.back());
	listenerAtSlot->index = listener->index;
	timeEventListeners.resize(timeEventListeners.size() - 1);
	listener->index = -1;
}

void EventBus::unregisterDataEventListener(DataEventListener *const listener) {
	if (listener->index == -1) return;
	auto &listenerList = dataEventListenerMap[listener->dataPath];
	auto &listenerAtSlot = listenerList[listener->index];
	listenerAtSlot = std::move(listenerList.back());
	listenerAtSlot->index = listener->index;
	listenerList.resize(listenerList.size() - 1);
	listener->index = -1;
}

void EventBus::notifyTimeEvent(const int timePassed) {
	for (const auto &listener : timeEventListeners) {
		listener->callback(timePassed);
	}
}

void EventBus::notifyDataEvent(const std::string &dataPath, const JSON &json) {
	auto entry = dataEventListenerMap.find(dataPath);
	if (entry == dataEventListenerMap.end()) return;
	for (const auto &listener : entry->second) {
		listener->callback(json);
	}
}

} // namespace CsgoHud