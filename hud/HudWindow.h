#ifndef CSGO_HUD_HUD_HUDWINDOW_H
#define CSGO_HUD_HUD_HUDWINDOW_H

#include <chrono>
#include <memory>

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
		CommonResources &commonResources;
		winrt::com_ptr<IDXGISwapChain4> swapChain;
		winrt::com_ptr<IDCompositionTarget> compositionTarget;

		JSON::dom::parser jsonParser;

		std::chrono::time_point<std::chrono::steady_clock> firstTick;

		LRESULT handleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
		void tick();
		void paint();
	public:
		static void preInitialize(HINSTANCE appInstance);

		std::unique_ptr<Component> mainComponent;

		HudWindow(HINSTANCE appInstance, CommonResources &commonResources);
		~HudWindow();
		HWND getWindowHandle();
		void update();
};

} // namespace CsgoHud

#endif // CSGO_HUD_HUD_HUDWINDOW_H