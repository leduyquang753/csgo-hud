#ifndef CSGO_HUD_DATAEVENTLISTENER_H
#define CSGO_HUD_DATAEVENTLISTENER_H

#include <cstdlib>
#include <functional>
#include <string>

#include "pch.h"

namespace CsgoHud {

struct DataEventListener final {
	private:
		friend class EventBus;
		EventBus *eventBus;
		// The JSON data path the listener listens to.
		std::string dataPath;
		// The index of the listener in the containing listener storage vector.
		std::size_t index;

		DataEventListener(EventBus *eventBus, const std::string &dataPath, std::size_t index);
	public:
		DataEventListener(const DataEventListener &other) = delete;
		DataEventListener(DataEventListener &&other);
		DataEventListener& operator=(const DataEventListener &other) = delete;
		DataEventListener& operator=(DataEventListener &&other);
		~DataEventListener();
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATAEVENTLISTENER_H