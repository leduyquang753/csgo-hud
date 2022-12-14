#include "pch.h"

#include "events/EventBus.h"

#include "events/KeyEventHook.h"

namespace CsgoHud {

namespace KeyEventHook {
	static HHOOK hook;
	static EventBus *eventBus;
	static bool keysEnabled = true;
	static DWORD toggleKey;

	LRESULT CALLBACK handleKeyboardEvent(const int code, const WPARAM message, const LPARAM lParam) {
		if (code == HC_ACTION && message == WM_KEYDOWN) {
			const auto keyCode = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam)->vkCode;
			if (keyCode == toggleKey)
				keysEnabled = !keysEnabled;
			else if (keysEnabled)
				eventBus->notifyKeyEvent(keyCode);
		}
		return CallNextHookEx(nullptr, code, message, lParam);
	}

	void registerHook(EventBus &eventBusIn, const DWORD toggleKeyIn) {
		eventBus = &eventBusIn;
		hook = SetWindowsHookEx(WH_KEYBOARD_LL, handleKeyboardEvent, nullptr, 0);
		toggleKey = toggleKeyIn;
	}

	void unregisterHook() {
		UnhookWindowsHookEx(hook);
		hook = nullptr;
		eventBus = nullptr;
	}
}

} // namespace CsgoHud