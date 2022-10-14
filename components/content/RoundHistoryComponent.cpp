#include <string>
#include <string_view>

#include "pch.h"

#include "components/base/Component.h"
#include "data/IconStorage.h"
#include "data/RoundsData.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"

#include "components/content/RoundHistoryComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == RoundHistoryComponent ==

RoundHistoryComponent::RoundHistoryComponent(CommonResources &commonResources, const TransitionedValue &transition):
	Component(commonResources),
	transition(transition),
	ctColor{0.35f, 0.72f, 0.96f, 1}, tColor{0.94f, 0.79f, 0.25f, 1}
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.7f}, backgroundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.5f}, inactiveRoundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, activeRoundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.3f}, highlightBrush.put());
	renderTarget.CreateSolidColorBrush(ctColor, ctBrush.put());
	renderTarget.CreateSolidColorBrush(tColor, tBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	renderTarget.CreateLayer(layer.put());
	
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	commonResources.writeFactory->CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	titleTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	textFormat = nullptr;
	commonResources.writeFactory->CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		20, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	numberTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
}

void RoundHistoryComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = transition.getValue();
	if (transitionValue == 0) return;

	const int firstPlayerIndex = commonResources.players.getFirstPlayerIndex();
	if (firstPlayerIndex == -1) return;
	const bool leftTeam = commonResources.players[firstPlayerIndex]->team;
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	static const float
		SLOT_WIDTH = 20,
		COUNT_WIDTH = 40,
		TITLE_HEIGHT = 20,
		SLOT_HEIGHT = 24,
		TOTAL_HEIGHT = (TITLE_HEIGHT + SLOT_HEIGHT) * 2,
		SPACING = 2;
	const float top = parentSize.height - TOTAL_HEIGHT;
	
	const bool transiting = transition.transiting();
	if (transiting) {
		renderTarget.PushLayer(
			{
				{0, 0, parentSize.width, parentSize.height},
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				1, nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			layer.get()
		);
		renderTarget.SetTransform(D2D1::Matrix3x2F::Translation(0, TOTAL_HEIGHT * (1-transitionValue)) * transform);
	}

	const auto &rounds = commonResources.rounds.getRounds();
	const int currentRound = commonResources.rounds.getCurrentRound();

	if (currentRound > 15 && currentRound < 31) {
		int leftScore = 0, rightScore = 0;
		for (int round = 0; round != 15; ++round)
			++(rounds[round].first == leftTeam ? rightScore : leftScore);

		const float
			left = (parentSize.width - (SLOT_WIDTH*15 + COUNT_WIDTH*2 + SPACING)) / 2,
			right = left + COUNT_WIDTH;
		renderTarget.FillRectangle({left, top, right, parentSize.height}, backgroundBrush.get());
		titleTextRenderer->draw(
			L"1ST"sv, {left, parentSize.height - TITLE_HEIGHT, right, parentSize.height}, textBrush
		);
		numberTextRenderer->draw(
			std::to_wstring(leftScore),
			{
				left, parentSize.height - (TITLE_HEIGHT + SLOT_HEIGHT*2),
				right, parentSize.height - (TITLE_HEIGHT + SLOT_HEIGHT)
			},
			leftTeam ? tBrush : ctBrush
		);
		numberTextRenderer->draw(
			std::to_wstring(rightScore),
			{left, parentSize.height - (TITLE_HEIGHT + SLOT_HEIGHT), right, parentSize.height - TITLE_HEIGHT},
			leftTeam ? ctBrush : tBrush
		);
	}

	const float
		left
			= currentRound < 16 ? (parentSize.width - (SLOT_WIDTH*15 + COUNT_WIDTH)) / 2
			: currentRound < 31 ? (
				(parentSize.width - (SLOT_WIDTH*15 + COUNT_WIDTH*2 + SPACING)) / 2 + COUNT_WIDTH + SPACING
			)
			: (parentSize.width - (SLOT_WIDTH*6 + COUNT_WIDTH)) / 2,
		right = left + SLOT_WIDTH * (currentRound < 31 ? 15 : 6) + COUNT_WIDTH,
		slotsStart = left + COUNT_WIDTH;
	renderTarget.FillRectangle({left, top, right, parentSize.height}, backgroundBrush.get());
	titleTextRenderer->draw(
		currentRound < 16 ? L"ROUND HISTORY – FIRST HALF"sv
		: currentRound < 31 ? L"ROUND HISTORY – SECOND HALF"sv
		: L"ROUND HISTORY – OVERTIME #"s + std::to_wstring((currentRound - 25) / 6),
		{left, parentSize.height - TITLE_HEIGHT, right, parentSize.height},
		textBrush
	);
	int leftScore = 0, rightScore = 0;
	const int
		startingRound
			= currentRound < 16 ? 0
			: currentRound < 31 ? 15
			: currentRound - (currentRound - 30) % 6,
		roundCount = currentRound < 31 ? 15 : 6;
	winrt::com_ptr<ID2D1SpriteBatch> spriteBatch;
	renderTarget.CreateSpriteBatch(spriteBatch.put());
	for (int i = 0, roundIndex = startingRound; i != roundCount; ++i, ++roundIndex) {
		const float
			slotLeft = slotsStart + SLOT_WIDTH*i,
			slotCenter = slotLeft + SLOT_WIDTH/2,
			slotRight = slotLeft + SLOT_WIDTH,
			topMiddle = parentSize.height - TITLE_HEIGHT - SLOT_HEIGHT * 3/2,
			bottomMiddle = parentSize.height - TITLE_HEIGHT - SLOT_HEIGHT/2;
		if (roundIndex + 1 == currentRound) renderTarget.FillRectangle(
			{slotLeft, top, slotRight, parentSize.height - TITLE_HEIGHT}, highlightBrush.get()
		);
		if (roundIndex % 5 == 4) titleTextRenderer->draw(
			std::to_wstring(roundIndex + 1), {slotLeft, top, slotRight, top + TITLE_HEIGHT}, textBrush
		);
		const auto roundBrush = roundIndex + 1 == currentRound ? activeRoundBrush.get() : inactiveRoundBrush.get();
		if (roundIndex < rounds.size()) {
			const auto &round = rounds[roundIndex];
			const bool leftTeamWon = (round.first == leftTeam) != (roundIndex > 29 && (roundIndex-30)%6 < 3);
			++(leftTeamWon ? leftScore : rightScore);
			const auto &icon = commonResources.icons[RoundsData::iconMap[static_cast<int>(round.second)]];
			const float
				iconHalfWidth = 8.f * icon.width / icon.height,
				iconMiddle = leftTeamWon ? topMiddle : bottomMiddle;
			const D2D1_RECT_F destinationRect = {
				slotCenter - iconHalfWidth, iconMiddle - 8, slotCenter + iconHalfWidth, iconMiddle + 8
			};
			spriteBatch->AddSprites(
				1, &destinationRect, &icon.bounds, round.first ? &ctColor : &tColor, nullptr, 0, 0, 0, 0
			);
			renderTarget.FillEllipse({{slotCenter, leftTeamWon ? bottomMiddle : topMiddle}, 3, 3}, roundBrush);
		} else {
			renderTarget.FillEllipse({{slotCenter, topMiddle}, 3, 3}, roundBrush);
			renderTarget.FillEllipse({{slotCenter, bottomMiddle}, 3, 3}, roundBrush);
		}
	}
	const auto oldMode = renderTarget.GetAntialiasMode();
	renderTarget.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	renderTarget.DrawSpriteBatch(
		spriteBatch.get(), commonResources.icons.getBitmap(),
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1_SPRITE_OPTIONS_NONE
	);
	renderTarget.SetAntialiasMode(oldMode);
	numberTextRenderer->draw(
		std::to_wstring(leftScore),
		{
			left, parentSize.height - (TITLE_HEIGHT + SLOT_HEIGHT*2),
			left + COUNT_WIDTH, parentSize.height - (TITLE_HEIGHT + SLOT_HEIGHT)
		},
		leftTeam ? ctBrush : tBrush
	);
	numberTextRenderer->draw(
		std::to_wstring(rightScore),
		{
			left, parentSize.height - (TITLE_HEIGHT + SLOT_HEIGHT),
			left + COUNT_WIDTH, parentSize.height - TITLE_HEIGHT
		},
		leftTeam ? tBrush : ctBrush
	);

	if (transiting) renderTarget.PopLayer();

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud