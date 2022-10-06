#ifndef CSGO_HUD_EVENTS_KEYEVENTHOOK_H
#define CSGO_HUD_EVENTS_KEYEVENTHOOK_H

namespace CsgoHud {

class EventBus;

namespace KeyEventHook {
	void registerHook(EventBus &eventBusIn);
	void unregisterHook();
}

} // namespace CsgoHud

#endif // CSGO_HUD_EVENTS_KEYEVENTHOOK_H