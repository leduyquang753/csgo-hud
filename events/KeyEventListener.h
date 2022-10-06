#ifndef CSGO_HUD_EVENTS_KEYEVENTLISTENER_H
#define CSGO_HUD_EVENTS_KEYEVENTLISTENER_H

#include <cstdlib>

#include "pch.h"

namespace CsgoHud {

struct KeyEventListener final {
	private:
		friend class EventBus;
		EventBus *eventBus;
		// The key code the listener listens to.
		WPARAM keyCode;
		// The index of the listener in the containing listener storage vector.
		std::size_t index;
		// The slot in the storage vector to replace when moved.
		KeyEventListener **slot;

		KeyEventListener(EventBus *eventBus, WPARAM keyCode, std::size_t index, KeyEventListener **slot);
	public:
		KeyEventListener(const KeyEventListener &other) = delete;
		KeyEventListener(KeyEventListener &&other);
		KeyEventListener& operator=(const KeyEventListener &other) = delete;
		KeyEventListener& operator=(KeyEventListener &&other);
		void unregister();
};

} // namespace CsgoHud

#endif // CSGO_HUD_EVENTS_KEYEVENTLISTENER_H