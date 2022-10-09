#include "pch.h"

#include <cstdlib>
#include <utility>

#include "events/EventBus.h"

#include "events/KeyEventListener.h"

namespace CsgoHud {

// == KeyEventListener ==

KeyEventListener::KeyEventListener(EventBus *const eventBus, const DWORD keyCode, const std::size_t index):
	eventBus(eventBus), keyCode(keyCode), index(index)
{
	eventBus->updateKeyEventListenerSlot(keyCode, index, this);
}

KeyEventListener::KeyEventListener(KeyEventListener &&other):
	eventBus(other.eventBus), keyCode(other.keyCode), index(other.index)
{
	eventBus->updateKeyEventListenerSlot(keyCode, index, this);
	// Make sure something happens when the moved listener is used.
	other.eventBus = nullptr;
}

KeyEventListener& KeyEventListener::operator=(KeyEventListener &&other) {
	eventBus = other.eventBus;
	keyCode = other.keyCode;
	index = other.index;
	eventBus->updateKeyEventListenerSlot(keyCode, index, this);
	other.eventBus = nullptr;
	return *this;
}

KeyEventListener::~KeyEventListener() {
	if (index != -1) eventBus->updateKeyEventListenerSlot(keyCode, index, nullptr);
}

void KeyEventListener::unregister() {
	eventBus->unregisterKeyEventListener(*this);
}

} // namespace CsgoHud