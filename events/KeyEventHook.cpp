#include "pch.h"

#include "events/EventBus.h"

#include "events/KeyEventHook.h"

namespace CsgoHud {

namespace KeyEventHook {
	static HHOOK hook;
	static EventBus *eventBus;

	LRESULT CALLBACK handleKeyboardEvent(const int code, const WPARAM message, const LPARAM lParam) {
		if (code == HC_ACTION && message == WM_KEYDOWN)
			eventBus->notifyKeyEvent(reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode);
		return CallNextHookEx(nullptr, code, message, lParam);
	}

	void registerHook(EventBus &eventBusIn) {
		eventBus = &eventBusIn;
		hook = SetWindowsHookEx(WH_KEYBOARD_LL, handleKeyboardEvent, nullptr, 0);
		auto error = GetLastError();
	}

	void unregisterHook() {
		UnhookWindowsHookEx(hook);
		hook = nullptr;
		eventBus = nullptr;
	}
}

} // namespace CsgoHud