#include <memory>

#include "pch.h"

#include "components/base/BagComponent.h"
#include "components/base/Component.h"
#include "components/base/SizedComponent.h"
#include "components/content/ActivePlayerComponent.h"
#include "components/content/TeamBuyComponent.h"
#include "movement/CubicBezierMovementFunction.h"
#include "resources/CommonResources.h"

#include "components/content/BottomBarComponent.h"

namespace CsgoHud {

// == BottomBarComponent ==

BottomBarComponent::BottomBarComponent(CommonResources &commonResources):
	Component(commonResources),
	transition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	)
{
	container = std::make_unique<BagComponent>(commonResources);
	container->children.emplace_back(std::make_unique<SizedComponent>(
		commonResources,
		D2D1_SIZE_F{0.3f, 80}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_PIXELS},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		std::make_unique<TeamBuyComponent>(commonResources, transition)
	));
	container->children.emplace_back(std::make_unique<SizedComponent>(
		commonResources,
		D2D1_SIZE_F{0.5f, 74}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_PIXELS},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		std::make_unique<ActivePlayerComponent>(commonResources, transition)
	));

	commonResources.eventBus.listenToKeyEvent('T', [this](){ onForceShowTeamBuyToggle(); });
}

void BottomBarComponent::onForceShowTeamBuyToggle() {
	forceShowTeamBuy = !forceShowTeamBuy;
}

void BottomBarComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const bool currentActivePlayerShown = !commonResources.rounds.isBeginningOfRound() && !forceShowTeamBuy;
	if (currentActivePlayerShown != activePlayerShown) {
		activePlayerShown = currentActivePlayerShown;
		transition.transition(activePlayerShown ? 1.f : 0.f);
	}
	container->paint(transform, parentSize);
}

} // namespace CsgoHud