#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "pch.h"

#include "components/base/BagComponent.h"
#include "components/base/PaddedComponent.h"
#include "components/content/FourCornersComponent.h"
#include "components/content/TestComponent.h"
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

	{
		std::vector<std::unique_ptr<CsgoHud::Component>> outerComponents;
		outerComponents.emplace_back(std::make_unique<CsgoHud::FourCornersComponent>(commonResources));
		outerComponents.emplace_back(std::make_unique<CsgoHud::PaddedComponent>(commonResources, 8.f,
			std::make_unique<CsgoHud::TestComponent>(commonResources)
		));
		hudWindow.mainComponent = std::make_unique<CsgoHud::BagComponent>(
			commonResources, std::move(outerComponents)
		);
	}
	
	MSG message;
	while (GetMessage(&message, nullptr, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	commonResources.httpServer.stop();
	httpThread.join();

	return 0;
}