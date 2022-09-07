#include <memory>
#include <string>

#include "pch.h"

#include "components/base/BagComponent.h"
#include "components/base/BlankComponent.h"
#include "components/base/Component.h"
#include "components/base/SizedComponent.h"
#include "components/base/StackComponent.h"
#include "components/base/StackComponentChild.h"
#include "components/content/PlayerInfoComponent.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"

#include "components/content/AllPlayersComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == AllPlayersComponent ==

AllPlayersComponent::AllPlayersComponent(CommonResources &commonResources): Component(commonResources) {
	auto &renderTarget = *commonResources.renderTarget;

	const D2D1_COLOR_F
		backgroundInactiveColor = {0, 0, 0, 0.3f},
		backgroundActiveColor = {0, 0, 0, 0.7f},
		activeOutlineColor = {1, 1, 1, 1};
	
	winrt::com_ptr<ID2D1SolidColorBrush> teamCtBrush, teamTBrush, healthBrush, textWhiteBrush, textGreenBrush;
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, teamCtBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, teamTBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0, 0, 1}, healthBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({0.5f, 1, 0.5f, 1}, textGreenBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> normalTextFormat, boldTextFormat;
	winrt::com_ptr<IDWriteInlineObject> trimmingSign;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", normalTextFormat.put()
	);
	writeFactory.CreateEllipsisTrimmingSign(normalTextFormat.get(), trimmingSign.put());
	static const DWRITE_TRIMMING TRIMMING_OPTIONS = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
	normalTextFormat->SetTrimming(&TRIMMING_OPTIONS, trimmingSign.get());
	normalTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
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

	container = std::make_unique<BagComponent>(commonResources);

	auto makeSide = [
		this,
		backgroundInactiveColor, backgroundActiveColor, activeOutlineColor,
		teamCtBrush, teamTBrush, healthBrush, textWhiteBrush, textGreenBrush,
		normalTextFormat, boldTextFormat
	](const int startIndex, const float anchor) {
		std::unique_ptr<StackComponent> currentStack;

		currentStack = std::make_unique<StackComponent>(
			this->commonResources,
			StackComponent::AXIS_VERTICAL, StackComponent::AXIS_DECREASE, 0.f, StackComponent::MODE_PIXELS
		);
		for (int i = 0; i != 5; ++i) {
			if (i != 0) {
				currentStack->children.emplace_back(StackComponentChild{
					{1, 5}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
					0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
					nullptr
				});
			}
			children[startIndex+4-i] = static_cast<PlayerInfoComponent*>(
				currentStack->children.emplace_back(StackComponentChild{
					{1, 40}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
					0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
					std::make_unique<PlayerInfoComponent>(
						this->commonResources,
						backgroundInactiveColor, backgroundActiveColor, activeOutlineColor,
						teamCtBrush, teamTBrush, healthBrush, textWhiteBrush, textGreenBrush,
						normalTextFormat, boldTextFormat, *normalTextRenderer, *boldTextRenderer
					)
				}).component.get()
			);
		}
		container->children.emplace_back(std::make_unique<SizedComponent>(
			this->commonResources,
			D2D1_SIZE_F{0.2f, 1.f}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			D2D1_POINT_2F{anchor, 1.f}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			D2D1_POINT_2F{anchor, 1.f}, D2D1_POINT_2U{SizedComponent::MODE_RATIO, SizedComponent::MODE_RATIO},
			std::move(currentStack)
		));
	};
	
	makeSide(0, 0);
	makeSide(5, 1);

	commonResources.eventBus.listenToDataEvent("player"s, [this](const JSON &json) { receivePlayerData(json); });
}

void AllPlayersComponent::receivePlayerData(const JSON &json) {
	activeSlot = json.contains("observer_slot"s) ? json["observer_slot"s].get<int>() : -1;
	if (activeSlot != -1) {
		if (activeSlot == 0) activeSlot = 9;
		else --activeSlot;
	}
}

void AllPlayersComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const auto &players = commonResources.players;
	int firstPlayerIndex = players.getFirstPlayerIndex();
	if (firstPlayerIndex == -1) return;
	const bool leftTeam = players[firstPlayerIndex]->team;
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
	
	container->paint(transform, parentSize);
}

} // namespace CsgoHud