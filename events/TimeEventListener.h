#ifndef CSGO_HUD_TIMEEVENTLISTENER_H
#define CSGO_HUD_TIMEEVENTLISTENER_H

#include <cstdlib>
#include <functional>

namespace CsgoHud {

struct TimeEventListener final {
	private:
		friend class EventBus;
		EventBus *const eventBus;
		// The index of the listener in the listener storage vector.
		std::size_t index;
		const std::function<void(int)> callback;

		TimeEventListener(EventBus *eventBus, std::size_t index, const std::function<void(int)> &callback);
	public:
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_TIMEEVENTLISTENER_H