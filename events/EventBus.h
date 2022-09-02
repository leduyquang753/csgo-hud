#ifndef CSGO_HUD_EVENTS_EVENTBUS_H
#define CSGO_HUD_EVENTS_EVENTBUS_H

#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "pch.h"

#include "events/TimeEventListener.h"
#include "events/DataEventListener.h"

namespace CsgoHud {

class EventBus final {
	private:
		std::vector<std::unique_ptr<TimeEventListener>> timeEventListeners;
		std::unordered_map<std::string, std::vector<std::unique_ptr<DataEventListener>>> dataEventListenerMap;

	public:
		TimeEventListener* listenToTimeEvent(const std::function<void(int)> &callback);
		DataEventListener* listenToDataEvent(
			const std::string &dataPath, const std::function<void(const JSON&)> &callback
		);
		void unregisterTimeEventListener(TimeEventListener *listener);
		void unregisterDataEventListener(DataEventListener *listener);
		void notifyTimeEvent(int timePassed);
		void notifyDataEvent(const std::string &dataPath, const JSON &json);
};

} // namespace CsgoHud

#endif // CSGO_HUD_EVENTS_EVENTBUS_H