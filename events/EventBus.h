#ifndef CSGO_HUD_EVENTS_EVENTBUS_H
#define CSGO_HUD_EVENTS_EVENTBUS_H

#include <cstdlib>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "pch.h"

#include "events/DataEventListener.h"
#include "events/KeyEventListener.h"
#include "events/TimeEventListener.h"

namespace CsgoHud {

class EventBus final {
	private:
		std::vector<std::pair<std::function<void(int)>, TimeEventListener*>> timeEventListeners;
		std::unordered_map<std::string, std::vector<std::pair<
			std::function<void(JSON::dom::object&)>, DataEventListener*
		>>> dataEventListenerMap;
		std::unordered_map<DWORD, std::vector<std::pair<std::function<void()>, KeyEventListener*>>>
			keyEventListenerMap;

	public:
		TimeEventListener listenToTimeEvent(const std::function<void(int)> &callback);
		DataEventListener listenToDataEvent(
			const std::string &dataPath, const std::function<void(JSON::dom::object&)> &callback
		);
		KeyEventListener listenToKeyEvent(DWORD keyCode, const std::function<void()> &callback);
		void updateTimeEventListenerSlot(std::size_t index, TimeEventListener *listener);
		void updateDataEventListenerSlot(
			const std::string &dataPath, std::size_t index, DataEventListener *listener
		);
		void updateKeyEventListenerSlot(DWORD keyCode, std::size_t index, KeyEventListener *listener);
		void unregisterTimeEventListener(TimeEventListener &listener);
		void unregisterDataEventListener(DataEventListener &listener);
		void unregisterKeyEventListener(KeyEventListener &listener);
		void notifyTimeEvent(int timePassed) const;
		void notifyDataEvent(const std::string &dataPath, JSON::dom::object &json) const;
		void notifyKeyEvent(DWORD keyCode) const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_EVENTS_EVENTBUS_H