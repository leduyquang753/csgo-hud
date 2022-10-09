#include "pch.h"

#include <string>

#include "components/base/Component.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/CommonConstants.h"

#include "components/content/RoundComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == RoundComponent ==

RoundComponent::RoundComponent(CommonResources &commonResources): Component(commonResources) {
	commonResources.renderTarget->CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	commonResources.writeFactory->CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	textRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
}

void RoundComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	commonResources.renderTarget->SetTransform(transform);
	int currentRound = commonResources.rounds.getCurrentRound();
	int overtime = 0;
	if (currentRound > 30) {
		overtime = (currentRound - 25) / 6;
		currentRound -= 24 + overtime * 6;
	}
	if (overtime == 0) {
		textRenderer->draw(
			L"Round "s + std::to_wstring(currentRound) + L" / 30"s,
			{0, 0, parentSize.width, 14},
			textBrush
		);
	} else {
		textRenderer->draw(
			L"Overtime #"s + std::to_wstring(overtime),
			{0, 0, parentSize.width, 14},
			textBrush
		);
		textRenderer->draw(
			L"Round "s + std::to_wstring(currentRound) + L" / 6"s,
			{0, 16, parentSize.width, 30},
			textBrush
		);
	}
	commonResources.renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud