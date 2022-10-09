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
	return {this, index};
}

DataEventListener EventBus::listenToDataEvent(
	const std::string &dataPath, const std::function<void(JSON::dom::object&)> &callback
) {
	auto &listenerList = dataEventListenerMap[dataPath];
	const std::size_t index = listenerList.size();
	listenerList.emplace_back(callback, nullptr);
	return {this, dataPath, index};
}

KeyEventListener EventBus::listenToKeyEvent(const DWORD keyCode, const std::function<void()> &callback) {
	auto &listenerList = keyEventListenerMap[keyCode];
	const std::size_t index = listenerList.size();
	listenerList.emplace_back(callback, nullptr);
	return {this, keyCode, index};
}

void EventBus::updateTimeEventListenerSlot(const std::size_t index, TimeEventListener *const listener) {
	timeEventListeners[index].second = listener;
}

void EventBus::updateDataEventListenerSlot(
	const std::string &dataPath, const std::size_t index, DataEventListener *const listener
) {
	dataEventListenerMap[dataPath][index].second = listener;
}

void EventBus::updateKeyEventListenerSlot(
	const DWORD keyCode, const std::size_t index, KeyEventListener *const listener
) {
	keyEventListenerMap[keyCode][index].second = listener;
}

void EventBus::unregisterTimeEventListener(TimeEventListener &listener) {
	if (listener.index == -1) return;
	auto &listenerAtSlot = timeEventListeners[listener.index];
	listenerAtSlot = std::move(timeEventListeners.back());
	if (listenerAtSlot.second != nullptr) listenerAtSlot.second->index = listener.index;
	timeEventListeners.pop_back();
	listener.index = -1;
}

void EventBus::unregisterDataEventListener(DataEventListener &listener) {
	if (listener.index == -1) return;
	auto &listenerList = dataEventListenerMap[listener.dataPath];
	auto &listenerAtSlot = listenerList[listener.index];
	listenerAtSlot = std::move(listenerList.back());
	if (listenerAtSlot.second != nullptr) listenerAtSlot.second->index = listener.index;
	listenerList.pop_back();
	listener.index = -1;
}

void EventBus::unregisterKeyEventListener(KeyEventListener &listener) {
	if (listener.index == -1) return;
	auto &listenerList = keyEventListenerMap[listener.keyCode];
	auto &listenerAtSlot = listenerList[listener.index];
	listenerAtSlot = std::move(listenerList.back());
	if (listenerAtSlot.second != nullptr) listenerAtSlot.second->index = listener.index;
	listenerList.pop_back();
	listener.index = -1;
}

void EventBus::notifyTimeEvent(const int timePassed) const {
	for (const auto &listener : timeEventListeners) listener.first(timePassed);
}

void EventBus::notifyDataEvent(const std::string &dataPath, JSON::dom::object &json) const {
	auto entry = dataEventListenerMap.find(dataPath);
	if (entry == dataEventListenerMap.end()) return;
	for (const auto &listener : entry->second) listener.first(json);
}

// https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms644984(v=vs.85)
void EventBus::notifyKeyEvent(const DWORD keyCode) const {
	auto entry = keyEventListenerMap.find(keyCode);
	if (entry == keyEventListenerMap.end()) return;
	for (const auto &listener : entry->second) listener.first();
}

} // namespace CsgoHud