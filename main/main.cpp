#include <memory>
#include <utility>
#include <vector>

#include "pch.h"

#include "components/base/BagComponent.h"
#include "components/base/Component.h"
#include "components/base/PaddedComponent.h"
#include "components/base/SizedComponent.h"
#include "components/base/StackComponent.h"
#include "components/base/StackComponentChild.h"
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
		std::vector<std::unique_ptr<CsgoHud::Component>> outerComponents;
		outerComponents.emplace_back(std::move(std::make_unique<CsgoHud::FourCornersComponent>(commonResources)));
		std::vector<std::unique_ptr<CsgoHud::Component>> innerComponents;
		innerComponents.emplace_back(std::move(std::make_unique<CsgoHud::FourCornersComponent>(commonResources)));
		innerComponents.emplace_back(std::move(std::make_unique<CsgoHud::TestComponent>(commonResources)));
		outerComponents.emplace_back(std::move(std::make_unique<CsgoHud::PaddedComponent>(commonResources, 16.f,
			std::move(std::make_unique<CsgoHud::BagComponent>(commonResources, std::move(innerComponents)))
		)));
		std::vector<CsgoHud::StackComponentChild> stackItems;
		stackItems.emplace_back(std::move(CsgoHud::StackComponentChild{
			{1, 100}, {true, false}, 0.5f, true, 0, false,
			std::move(std::make_unique<CsgoHud::FourCornersComponent>(commonResources))
		}));
		stackItems.emplace_back(std::move(CsgoHud::StackComponentChild{
			{1, 0.1f}, {true, true}, 0.5f, true, 10, false,
			std::move(std::make_unique<CsgoHud::FourCornersComponent>(commonResources))
		}));
		outerComponents.emplace_back(std::move(std::make_unique<CsgoHud::SizedComponent>(
			commonResources,
			D2D1_SIZE_F{200, 0.8f}, D2D1_POINT_2U{false, true},
			D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{true, true},
			D2D1_POINT_2F{0.4f, 1}, D2D1_POINT_2U{true, true},
			std::move(std::make_unique<CsgoHud::StackComponent>(
				commonResources, true, true, 0.5f, true, std::move(stackItems)
			))
		)));
		hudWindow.mainComponent = std::move(std::make_unique<CsgoHud::BagComponent>(
			commonResources, std::move(outerComponents)
		));
	}
	
	MSG message;
	while (GetMessage(&message, nullptr, 0, 0) > 0) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}