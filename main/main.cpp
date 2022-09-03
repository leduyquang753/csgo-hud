#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "pch.h"

#include "components/content/HudComponent.h"
#include "hud/HudWindow.h"
#include "resources/CommonResources.h"

int WINAPI wWinMain(
	const HINSTANCE instance, const HINSTANCE previousInstance, const PWSTR commandLine, const int showFlag
) {
	// Allocate dynamically as the object is potentially large.
	auto commonResourcesPointer = std::make_unique<CsgoHud::CommonResources>();
	auto &commonResources = *commonResourcesPointer;
	
	CsgoHud::HudWindow::preInitialize(instance);
	CsgoHud::HudWindow hudWindow(instance, commonResources);

	std::thread httpThread([&]() {
		commonResources.httpServer.run();
	});

	hudWindow.mainComponent = std::make_unique<CsgoHud::HudComponent>(commonResources);
	
	MSG message;
	while (GetMessage(&message, nullptr, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	commonResources.httpServer.stop();
	httpThread.join();

	return 0;
}