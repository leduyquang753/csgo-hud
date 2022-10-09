#ifndef CSGO_HUD_EVENTS_KEYEVENTLISTENER_H
#define CSGO_HUD_EVENTS_KEYEVENTLISTENER_H

#include <cstdlib>

namespace CsgoHud {

struct KeyEventListener final {
	private:
		friend class EventBus;
		EventBus *eventBus;
		// The key code the listener listens to.
		DWORD keyCode;
		// The index of the listener in the containing listener storage vector.
		std::size_t index;

		KeyEventListener(EventBus *eventBus, DWORD keyCode, std::size_t index);
	public:
		KeyEventListener(const KeyEventListener &other) = delete;
		KeyEventListener(KeyEventListener &&other);
		KeyEventListener& operator=(const KeyEventListener &other) = delete;
		KeyEventListener& operator=(KeyEventListener &&other);
		~KeyEventListener();
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_EVENTS_KEYEVENTLISTENER_H