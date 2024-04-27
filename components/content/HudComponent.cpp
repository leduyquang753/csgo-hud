#include "pch.h"

#include <memory>
#include <string_view>
#include <utility>

#include "components/base/Component.h"
#include "components/base/BagComponent.h"
#include "components/base/PaddedComponent.h"
#include "components/base/SizedComponent.h"
#include "components/content/AllPlayersComponent.h"
#include "components/content/BottomBarComponent.h"
#include "components/content/FourCornersComponent.h"
#include "components/content/FpsComponent.h"
#include "components/content/MinimapComponent.h"
#include "components/content/RoundComponent.h"
#include "components/content/TopBarComponent.h"
#include "resources/CommonResources.h"
#include "utils/Utils.h"

#include "components/content/HudComponent.h"

using namespace std::string_view_literals;

namespace CsgoHud {

struct CommonResources;

// == HudComponent ==

HudComponent::HudComponent(CommonResources &commonResources):
	Component(commonResources), bag(std::make_unique<BagComponent>(commonResources))
{
	bag->children.emplace_back(std::make_unique<FourCornersComponent>(commonResources));
	auto innerBag = std::make_unique<BagComponent>(commonResources);
	innerBag->children.emplace_back(std::make_unique<SizedComponent>(
		commonResources,
		D2D1_SIZE_F{0.25f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		D2D1_POINT_2F{0, 0}, D2D1_POINT_2U{SizedComponent::MODE_PIXELS, SizedComponent::MODE_PIXELS},
		D2D1_POINT_2F{0, 0}, D2D1_POINT_2U{SizedComponent::MODE_PIXELS, SizedComponent::MODE_PIXELS},
		std::make_unique<MinimapComponent>(commonResources)
	));
	innerBag->children.emplace_back(std::make_unique<RoundComponent>(commonResources));
	innerBag->children.emplace_back(std::make_unique<TopBarComponent>(commonResources));
	innerBag->children.emplace_back(std::make_unique<BottomBarComponent>(commonResources));
	innerBag->children.emplace_back(std::make_unique<AllPlayersComponent>(commonResources));
	innerBag->children.emplace_back(std::make_unique<FpsComponent>(commonResources));
	bag->children.emplace_back(
		std::make_unique<PaddedComponent>(commonResources, 8.f, std::move(innerBag))
	);

	commonResources.eventBus.listenToKeyEvent(
		Utils::parseKeyCode(commonResources.configuration.keybindings["toggleHud"sv].value().get_string().value()),
		[this](){ onVisibilityToggle(); }
	);
}

void HudComponent::onVisibilityToggle() {
	shown = !shown;
}

void HudComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	if (!shown) return;
	bag->paint(transform, parentSize);
}

} // namespace CsgoHud