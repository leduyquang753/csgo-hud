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

		TimeEventListener(EventBus *eventBus, std::size_t index);
	public:
		TimeEventListener(const TimeEventListener &other) = delete;
		TimeEventListener(TimeEventListener &&other);
		TimeEventListener& operator=(const TimeEventListener &other) = delete;
		TimeEventListener& operator=(TimeEventListener &&other);
		~TimeEventListener();
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_TIMEEVENTLISTENER_H