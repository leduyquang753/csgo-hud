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
		EventBus *const eventBus;
		// The JSON data path the listener listens to.
		const std::string dataPath;
		// The index of the listener in the containing listener storage vector.
		std::size_t index;
		const std::function<void(const JSON&)> callback;

		DataEventListener(
			EventBus *eventBus, const std::string &dataPath, std::size_t index,
			const std::function<void(const JSON&)> &callback
		);
	public:
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATAEVENTLISTENER_H