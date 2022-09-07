#include <cstdlib>
#include <memory>
#include <functional>
#include <utility>

#include "pch.h"

#include "events/TimeEventListener.h"
#include "events/DataEventListener.h"

#include "events/EventBus.h"

namespace CsgoHud {

// == EventBus ==

TimeEventListener EventBus::listenToTimeEvent(const std::function<void(int)> &callback) {
	const std::size_t index = timeEventListeners.size();
	timeEventListeners.emplace_back(callback, nullptr);
	auto &slot = timeEventListeners.back().second;
	TimeEventListener listener(this, index, &slot);
	slot = &listener;
	return listener;
}

DataEventListener EventBus::listenToDataEvent(
	const std::string &dataPath, const std::function<void(const JSON&)> &callback
) {
	auto &listenerList = dataEventListenerMap[dataPath];
	const std::size_t index = listenerList.size();
	listenerList.emplace_back(callback, nullptr);
	auto &slot = listenerList.back().second;
	DataEventListener listener(this, dataPath, index, &slot);
	slot = &listener;
	return listener;
}

void EventBus::unregisterTimeEventListener(TimeEventListener &listener) {
	if (listener.index == -1) return;
	auto &listenerAtSlot = timeEventListeners[listener.index];
	listenerAtSlot = std::move(timeEventListeners.back());
	listenerAtSlot.second->index = listener.index;
	timeEventListeners.pop_back();
	listener.index = -1;
}

void EventBus::unregisterDataEventListener(DataEventListener &listener) {
	if (listener.index == -1) return;
	auto &listenerList = dataEventListenerMap[listener.dataPath];
	auto &listenerAtSlot = listenerList[listener.index];
	listenerAtSlot = std::move(listenerList.back());
	listenerAtSlot.second->index = listener.index;
	listenerList.pop_back();
	listener.index = -1;
}

void EventBus::notifyTimeEvent(const int timePassed) const {
	for (const auto &listener : timeEventListeners) {
		listener.first(timePassed);
	}
}

void EventBus::notifyDataEvent(const std::string &dataPath, const JSON &json) const {
	auto entry = dataEventListenerMap.find(dataPath);
	if (entry == dataEventListenerMap.end()) return;
	for (const auto &listener : entry->second) {
		listener.first(json);
	}
}

} // namespace CsgoHud