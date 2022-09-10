#ifndef CSGO_HUD_HUD_HUDWINDOW_H
#define CSGO_HUD_HUD_HUDWINDOW_H

#include <chrono>
#include <memory>

#include "pch.h"

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;

class HudWindow final {
	private:
		friend static LRESULT CALLBACK receiveWindowMessage(
			HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam
		);
		static LRESULT CALLBACK receiveWindowMessage(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);
	
		HWND windowHandle;
		HDC windowSurface;
		HDC renderSurface;
		CommonResources &commonResources;

		JSON::dom::parser jsonParser;

		std::chrono::time_point<std::chrono::steady_clock> firstTick;
		int lastTickTime = -1;

		LRESULT handleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
		void tick();
		void paint();
	public:
		static void preInitialize(HINSTANCE appInstance);
		
		std::unique_ptr<Component> mainComponent;
		
		HudWindow(HINSTANCE appInstance, CommonResources &commonResources);
		~HudWindow();
		HWND getWindowHandle();
};

} // namespace CsgoHud

#endif // CSGO_HUD_HUD_HUDWINDOW_H