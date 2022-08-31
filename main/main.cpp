#include <memory>
#include <utility>

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
	auto commonResourcesPointer = std::make_unique<CsgoHud::CommonResources>();
	auto &commonResources = *commonResourcesPointer;
	CsgoHud::HudWindow::preInitialize(instance);
	CsgoHud::HudWindow hudWindow(instance, commonResources);

	{
		auto mainComponent = std::make_unique<CsgoHud::BagComponent>(commonResources);
		mainComponent->children.emplace_back(
			std::move(std::make_unique<CsgoHud::FourCornersComponent>(commonResources))
		);
		auto paddedComponent = static_cast<CsgoHud::PaddedComponent*>(mainComponent->children.emplace_back(
			std::move(std::make_unique<CsgoHud::PaddedComponent>(commonResources, 16.f))
		).get());
		auto innerBag = std::make_unique<CsgoHud::BagComponent>(commonResources);
		innerBag->children.emplace_back(
			std::move(std::make_unique<CsgoHud::FourCornersComponent>(commonResources))
		);
		innerBag->children.emplace_back(
			std::move(std::make_unique<CsgoHud::TestComponent>(commonResources))
		);
		paddedComponent->child = std::move(innerBag);
		hudWindow.mainComponent = std::move(mainComponent);
	}
	
	MSG message;
	while (GetMessage(&message, nullptr, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}