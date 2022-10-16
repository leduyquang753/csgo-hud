#include "pch.h"

#include <memory>
#include <string_view>

#include "components/base/BagComponent.h"
#include "components/base/Component.h"
#include "components/base/SizedComponent.h"
#include "components/content/ActivePlayerComponent.h"
#include "components/content/RoundHistoryComponent.h"
#include "components/content/TeamBuyComponent.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/MovementFunction.h"
#include "resources/CommonResources.h"
#include "utils/Utils.h"

#include "components/content/BottomBarComponent.h"

using namespace std::string_view_literals;

namespace CsgoHud {

static std::vector<std::unique_ptr<MovementFunction>> getSlidingMovementFunctions() {
	std::vector<std::unique_ptr<MovementFunction>> functions;
	functions.emplace_back(std::make_unique<CubicBezierMovementFunction>(
		std::vector<D2D1_POINT_2F>{{{0, 0}, {0, 0}, {0.58f, 1}, {150, 1}, {0, 0}, {1, 1}, {300, 1}}}
	));
	functions.emplace_back(std::make_unique<CubicBezierMovementFunction>(
		std::vector<D2D1_POINT_2F>{{{0, 0}, {0, 0}, {1, 1}, {150, 0}, {0, 0}, {0.58f, 1}, {300, 1}}}
	));
	return functions;
}

// == BottomBarComponent ==

BottomBarComponent::BottomBarComponent(CommonResources &commonResources):
	Component(commonResources),
	fadingTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	),
	slidingTransition1(commonResources, getSlidingMovementFunctions(), 300, 0),
	slidingTransition2(commonResources, getSlidingMovementFunctions(), 300, 0)
{
	container = std::make_unique<BagComponent>(commonResources);
	container->children.emplace_back(std::make_unique<SizedComponent>(
		commonResources,
		D2D1_SIZE_F{0.3f, 80}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_PIXELS},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		std::make_unique<TeamBuyComponent>(commonResources, fadingTransition, slidingTransition1)
	));
	container->children.emplace_back(std::make_unique<SizedComponent>(
		commonResources,
		D2D1_SIZE_F{0.5f, 74}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_PIXELS},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		D2D1_POINT_2F{0.5f, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
		std::make_unique<ActivePlayerComponent>(commonResources, fadingTransition, slidingTransition1)
	));
	container->children.emplace_back(std::make_unique<RoundHistoryComponent>(
		commonResources, slidingTransition2
	));

	commonResources.eventBus.listenToKeyEvent(
		Utils::parseKeyCode(
			commonResources.configuration.keybindings["toggleForceShowTeamBuy"sv].value().get_string().value()
		),
		[this](){ onForceShowTeamBuyToggle(); }
	);
	commonResources.eventBus.listenToKeyEvent(
		Utils::parseKeyCode(
			commonResources.configuration.keybindings["toggleRoundHistory"sv].value().get_string().value()
		),
		[this](){ onShowRoundHistoryToggle(); }
	);
}

void BottomBarComponent::onForceShowTeamBuyToggle() {
	forceShowTeamBuy = !forceShowTeamBuy;
}

void BottomBarComponent::onShowRoundHistoryToggle() {
	showRoundHistory = !showRoundHistory;
	if (showRoundHistory) {
		slidingTransition1.transition(1, 0);
		slidingTransition2.transition(1, 1);
	} else {
		slidingTransition1.transition(0, 1);
		slidingTransition2.transition(0, 0);
	}
}

void BottomBarComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const bool currentActivePlayerShown = !commonResources.rounds.isBeginningOfRound() && !forceShowTeamBuy;
	if (currentActivePlayerShown != activePlayerShown) {
		activePlayerShown = currentActivePlayerShown;
		fadingTransition.transition(activePlayerShown ? 1.f : 0.f);
	}
	container->paint(transform, parentSize);
}

} // namespace CsgoHud