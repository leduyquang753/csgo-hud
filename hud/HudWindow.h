#ifndef CSGO_HUD_HUD_HUDWINDOW_H
#define CSGO_HUD_HUD_HUDWINDOW_H

#include <memory>

#include "pch.h"

#include "components/Component.h"

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

		LRESULT handleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
		void paint();
	public:
		static void preInitialize(HINSTANCE appInstance);
		
		std::unique_ptr<Component> mainComponent;
		
		HudWindow(HINSTANCE appInstance, CommonResources &commonResources);
		~HudWindow();
};

} // namespace CsgoHud

#endif // CSGO_HUD_HUD_HUDWINDOW_H