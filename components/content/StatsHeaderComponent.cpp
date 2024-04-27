#include "pch.h"

#include <string>

#include "components/base/Component.h"
#include "components/content/PlayerInfoComponent.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"

#include "components/content/StatsHeaderComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == StatsHeaderComponent ==

StatsHeaderComponent::StatsHeaderComponent(
	CommonResources &commonResources, const bool rightSide, const PlayerInfoComponent::Resources &resources
): Component(commonResources), rightSide(rightSide), resources(resources) {}

void StatsHeaderComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	static const float
		KD_CELL_LENGTH = 30,
		MONEY_CELL_LENGTH = 64,
		STAT_CELL_SPACING = 4,
		STATS_LENGTH = KD_CELL_LENGTH + MONEY_CELL_LENGTH + STAT_CELL_SPACING;

	const float statsTransitionValue = resources.statsTransition.getValue();
	if (statsTransitionValue == 0) return;
	const bool statsTransiting = resources.statsTransition.transiting();

	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	const float
		verticalMiddle = parentSize.height / 2,
		currentLength = STATS_LENGTH * statsTransitionValue;
	if (statsTransiting) renderTarget.PushLayer(
		{
			rightSide
				? D2D1_RECT_F{-STAT_CELL_SPACING - currentLength, 0, -STAT_CELL_SPACING, parentSize.height}
				: D2D1_RECT_F{
					parentSize.width + STAT_CELL_SPACING, 0,
					parentSize.width + STAT_CELL_SPACING + currentLength, parentSize.height
				},
			nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
			1, nullptr, D2D1_LAYER_OPTIONS_NONE
		},
		layer.get()
	);
	const float
		start = rightSide
			? -STAT_CELL_SPACING - currentLength
			: parentSize.width + STAT_CELL_SPACING - STATS_LENGTH + currentLength,
		kdStart = rightSide ? start + MONEY_CELL_LENGTH + STAT_CELL_SPACING : start,
		kdEnd = kdStart + KD_CELL_LENGTH,
		moneyStart = rightSide ? start : start + KD_CELL_LENGTH + STAT_CELL_SPACING,
		moneyEnd = moneyStart + MONEY_CELL_LENGTH;
	const auto teamBrush = ct ? resources.teamCtBrush.get() : resources.teamTBrush.get();
	renderTarget.FillRectangle({kdStart, 0, kdEnd, parentSize.height}, teamBrush);
	renderTarget.FillRectangle({moneyStart, 0, moneyEnd, parentSize.height}, teamBrush);
	resources.normalTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	resources.normalTextRenderer.draw(L"K"s, {kdStart, 0, kdEnd, verticalMiddle}, resources.textWhiteBrush);
	resources.normalTextRenderer.draw(
		L"D"s, {kdStart, verticalMiddle, kdEnd, parentSize.height}, resources.textWhiteBrush
	);
	resources.normalTextRenderer.draw(
		L"Buy"s, {moneyStart, 0, moneyEnd, verticalMiddle}, resources.textWhiteBrush
	);
	resources.normalTextRenderer.draw(
		L"Equip."s, {moneyStart, verticalMiddle, moneyEnd, parentSize.height}, resources.textWhiteBrush
	);
	if (statsTransiting) renderTarget.PopLayer();
}

} // namespace CsgoHud