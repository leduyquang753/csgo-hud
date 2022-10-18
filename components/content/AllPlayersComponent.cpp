#include "pch.h"

#include <memory>
#include <string>
#include <string_view>

#include "components/base/BagComponent.h"
#include "components/base/BlankComponent.h"
#include "components/base/Component.h"
#include "components/base/SizedComponent.h"
#include "components/base/StackComponent.h"
#include "components/base/StackComponentChild.h"
#include "components/content/PlayerInfoComponent.h"
#include "components/content/StatsHeaderComponent.h"
#include "components/content/UtilityComponent.h"
#include "data/RoundsData.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/MovementFunction.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/AllPlayersComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

static std::vector<std::unique_ptr<MovementFunction>> getMovementFunctions() {
	std::vector<std::unique_ptr<MovementFunction>> functions;
	functions.emplace_back(std::make_unique<CubicBezierMovementFunction>(
		std::vector<D2D1_POINT_2F>{{{0, 0}, {0, 0}, {0.58f, 1}, {300, 1}}}
	));
	functions.emplace_back(std::make_unique<CubicBezierMovementFunction>(
		std::vector<D2D1_POINT_2F>{{{0, 0}, {0.42f, 0}, {1, 1}, {300, 1}}}
	));
	return functions;
}

// == AllPlayersComponent ==

AllPlayersComponent::AllPlayersComponent(CommonResources &commonResources):
	Component(commonResources),
	statsTransition(commonResources, getMovementFunctions(), 300, 0),
	utilityTransition(commonResources, getMovementFunctions(), 300, 0)
{
	auto &renderTarget = *commonResources.renderTarget;
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> normalTextFormat, boldTextFormat;
	winrt::com_ptr<IDWriteInlineObject> trimmingSign;
	const auto fontFamily = commonResources.configuration.fontFamily.c_str();
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", normalTextFormat.put()
	);
	writeFactory.CreateEllipsisTrimmingSign(normalTextFormat.get(), trimmingSign.put());
	static const DWRITE_TRIMMING TRIMMING_OPTIONS = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
	normalTextFormat->SetTrimming(&TRIMMING_OPTIONS, trimmingSign.get());
	normalTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", boldTextFormat.put()
	);
	
	normalTextRenderer.emplace(
		commonResources, normalTextFormat,
		CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	boldTextRenderer.emplace(
		commonResources, boldTextFormat,
		CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	resources.emplace(PlayerInfoComponent::Resources{
		.backgroundInactiveColor = {0, 0, 0, 0.3f},
		.backgroundActiveColor = {0, 0, 0, 0.7f},
		.activeOutlineColor = {1, 1, 1, 1},
		.weaponInactiveColor = {1, 1, 1, 0.6f},
		.weaponEmptyActiveColor = {1, 0.5f, 0.5f, 1},
		.weaponEmptyInactiveColor = {1, 0.5f, 0.5f, 0.6f},
		.flashColor = {1, 1, 1, 0.5f},
		.smokeColor = {0.8f, 0.8f, 0.8f, 0.5f},
		.fireColor = {1, 0.6f, 0, 0.5f},
		.normalTextFormat = normalTextFormat,
		.boldTextFormat = boldTextFormat,
		.normalTextRenderer = *normalTextRenderer,
		.boldTextRenderer = *boldTextRenderer,
		.statsTransition = statsTransition
	});
	
	const auto &colors = commonResources.configuration.colors;
	renderTarget.CreateSolidColorBrush(colors.ctPrimary, resources->teamCtBrush.put());
	renderTarget.CreateSolidColorBrush(colors.tPrimary, resources->teamTBrush.put());
	renderTarget.CreateSolidColorBrush(colors.damage, resources->healthBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, resources->textWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({0.5f, 1, 0.5f, 1}, resources->textGreenBrush.put());

	container = std::make_unique<BagComponent>(commonResources);

	auto makeSide = [this](
		const int startIndex, const float anchor, StatsHeaderComponent *&header, UtilityComponent *&utility
	) {
		const bool rightSide = startIndex > 4;
		std::unique_ptr<StackComponent> currentStack;
		currentStack = std::make_unique<StackComponent>(
			this->commonResources,
			StackComponent::AXIS_VERTICAL, StackComponent::AXIS_DECREASE, 0.f, StackComponent::MODE_PIXELS
		);
		for (int i = 0; i != 5; ++i) {
			children[startIndex+4-i] = static_cast<PlayerInfoComponent*>(
				currentStack->children.emplace_back(StackComponentChild{
					{1, 40}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
					0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
					std::make_unique<PlayerInfoComponent>(this->commonResources, rightSide, *resources)
				}).component.get()
			);
			currentStack->children.emplace_back(StackComponentChild{
				{1, 5}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
				0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
				nullptr
			});
		}
		
		auto topArea = static_cast<BagComponent*>(	
			currentStack->children.emplace_back(StackComponentChild{
				{1, 40}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
				0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
				std::make_unique<BagComponent>(this->commonResources)
			}).component.get()
		);
		auto headerPointer
			= std::make_unique<StatsHeaderComponent>(this->commonResources, rightSide, *resources);
		header = headerPointer.get();
		auto utilityPointer
			= std::make_unique<UtilityComponent>(this->commonResources, rightSide, utilityTransition);
		utility = utilityPointer.get();
		topArea->children.emplace_back(std::make_unique<SizedComponent>(
			this->commonResources,
			D2D1_SIZE_F{1, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			D2D1_POINT_2F{0, 0}, D2D1_POINT_2U{SizedComponent::MODE_PIXELS, SizedComponent::MODE_PIXELS},
			D2D1_POINT_2F{0, 0}, D2D1_POINT_2U{SizedComponent::MODE_PIXELS, SizedComponent::MODE_PIXELS},
			std::move(utilityPointer)
		));
		topArea->children.emplace_back(std::make_unique<SizedComponent>(
			this->commonResources,
			D2D1_SIZE_F{1, 1}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			D2D1_POINT_2F{0, 0}, D2D1_POINT_2U{SizedComponent::MODE_PIXELS, SizedComponent::MODE_PIXELS},
			D2D1_POINT_2F{0, 0}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_PIXELS},
			std::move(headerPointer)
		));
		
		container->children.emplace_back(std::make_unique<SizedComponent>(
			this->commonResources,
			D2D1_SIZE_F{0.2f, 1.f}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			D2D1_POINT_2F{anchor, 1.f}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			D2D1_POINT_2F{anchor, 1.f}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			std::move(currentStack)
		));
	};
	
	makeSide(0, 0, leftStatsHeader, leftUtility);
	makeSide(5, 1, rightStatsHeader, rightUtility);
	
	commonResources.eventBus.listenToKeyEvent(
		Utils::parseKeyCode(
			commonResources.configuration.keybindings["toggleUtility"sv].value().get_string().value()
		),
		[this](){ onUtilityToggle(); }
	);
}

void AllPlayersComponent::onUtilityToggle() {
	utilityOn = !utilityOn;
	utilityTransition.transition(utilityOn ? 1.f : 0.f, utilityOn ? 0 : 1);
}

void AllPlayersComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const auto &players = commonResources.players;
	int firstPlayerIndex = players.getFirstPlayerIndex();
	if (firstPlayerIndex == -1) return;

	const bool currentStatsOn = commonResources.rounds.isBeginningOfRound();
	if (statsOn != currentStatsOn) {
		statsOn = currentStatsOn;
		statsTransition.transition(statsOn ? 1.f : 0.f, statsOn ? 0 : 1);
	}
	
	const bool leftTeam = players[firstPlayerIndex]->team;
	const int activeSlot = players.getActivePlayerIndex();
	int leftSideSlot = 0, rightSideSlot = 5;
	for (int i = 0; i != 10; ++i) {
		const auto &player = players[i];
		if (player) {
			PlayerInfoComponent *child;
			if (player->team == leftTeam) {
				child = children[leftSideSlot];
				++leftSideSlot;
			} else {
				child = children[rightSideSlot];
				++rightSideSlot;
			}
			child->index = i;
			child->active = i == activeSlot;
		}
	}
	for (int i = leftSideSlot; i != 5; ++i) children[i]->index = -1;
	for (int i = rightSideSlot; i != 10; ++i) children[i]->index = -1;
	leftStatsHeader->ct = leftTeam;
	rightStatsHeader->ct = !leftTeam;
	leftUtility->ct = leftTeam;
	rightUtility->ct = !leftTeam;
	
	container->paint(transform, parentSize);
}

} // namespace CsgoHud