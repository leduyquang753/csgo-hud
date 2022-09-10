#ifndef CSGO_HUD_EVENTS_EVENTBUS_H
#define CSGO_HUD_EVENTS_EVENTBUS_H

#include <cstdlib>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "pch.h"

#include "events/TimeEventListener.h"
#include "events/DataEventListener.h"

namespace CsgoHud {

class EventBus final {
	private:
		std::vector<std::pair<std::function<void(int)>, TimeEventListener*>> timeEventListeners;
		std::unordered_map<std::string, std::vector<std::pair<
			std::function<void(JSON::dom::object&)>, DataEventListener*
		>>> dataEventListenerMap;

	public:
		TimeEventListener listenToTimeEvent(const std::function<void(int)> &callback);
		DataEventListener listenToDataEvent(
			const std::string &dataPath, const std::function<void(JSON::dom::object&)> &callback
		);
		void unregisterTimeEventListener(TimeEventListener &listener);
		void unregisterDataEventListener(DataEventListener &listener);
		void notifyTimeEvent(int timePassed) const;
		void notifyDataEvent(const std::string &dataPath, JSON::dom::object &json) const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_EVENTS_EVENTBUS_H