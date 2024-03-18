#include "pch.h"

#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "components/content/HudComponent.h"
#include "data/AllPlayersData.h"
#include "data/IconStorage.h"
#include "hud/HudWindow.h"
#include "resources/CommonResources.h"

int WINAPI wWinMain(
	const HINSTANCE instance, const HINSTANCE previousInstance, const PWSTR commandLine, const int showFlag
) {
	CoInitialize(nullptr);

	{ // Block enclosing resources to be freed before returning.
	
	// Allocate dynamically as the object is potentially large.
	auto commonResourcesPointer = std::make_unique<CsgoHud::CommonResources>();
	auto &commonResources = *commonResourcesPointer;
	
	CsgoHud::HudWindow::preInitialize(instance);
	CsgoHud::HudWindow hudWindow(instance, commonResources);

	std::thread httpThread([&commonResources, &hudWindow]() {
		commonResources.httpServer.run(hudWindow.getWindowHandle(), commonResources.configuration.httpServerPort);
	});

	hudWindow.mainComponent = std::make_unique<CsgoHud::HudComponent>(commonResources);
	
	MSG message;
	while (true) {
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessage(&message);
			if (message.message == WM_QUIT) break;
		} else {
			hudWindow.update();
		}
	}

	commonResources.httpServer.stop();
	httpThread.join();
	
	} // Block enclosing resources to be freed before returning.

	CoUninitialize();

	return 0;
}