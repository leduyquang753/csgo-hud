#ifndef CSGO_HUD_TIMEEVENTLISTENER_H
#define CSGO_HUD_TIMEEVENTLISTENER_H

#include <cstdlib>
#include <functional>

namespace CsgoHud {

struct TimeEventListener final {
	private:
		friend class EventBus;
		EventBus *eventBus;
		// The index of the listener in the listener storage vector.
		std::size_t index;
		// The slot in the storage vector to replace when moved.
		TimeEventListener **slot;

		TimeEventListener(EventBus *eventBus, std::size_t index, TimeEventListener **slot);
	public:
		TimeEventListener(const TimeEventListener &other) = delete;
		TimeEventListener(TimeEventListener &&other);
		TimeEventListener& operator=(const TimeEventListener &other) = delete;
		TimeEventListener& operator=(TimeEventListener &&other);
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_TIMEEVENTLISTENER_H