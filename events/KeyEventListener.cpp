#include <cstdlib>
#include <utility>

#include "pch.h"

#include "events/EventBus.h"

#include "events/KeyEventListener.h"

namespace CsgoHud {

// == KeyEventListener ==

KeyEventListener::KeyEventListener(
	EventBus *const eventBus, const WPARAM keyCode, const std::size_t index, KeyEventListener **const slot
): eventBus(eventBus), keyCode(keyCode), index(index), slot(slot) {}

KeyEventListener::KeyEventListener(KeyEventListener &&other):
	eventBus(other.eventBus), keyCode(other.keyCode), index(other.index), slot(other.slot)
{
	*slot = this;
	// Make sure something happens when the moved listener is used.
	other.eventBus = nullptr;
}

KeyEventListener& KeyEventListener::operator=(KeyEventListener &&other) {
	eventBus = other.eventBus;
	keyCode = other.keyCode;
	index = other.index;
	slot = other.slot;
	*slot = this;
	other.eventBus = nullptr;
	return *this;
}

} // namespace CsgoHud