#include "pch.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "components/base/Component.h"
#include "data/BombData.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/BombTimerComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == BombTimerComponent ==

BombTimerComponent::BombTimerComponent(CommonResources &commonResources):
	Component(commonResources),
	bombTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	),
	defuseTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	)
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, gaugeOuterBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.1f}, gaugeInnerPlantingBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.3f}, gaugeInnerPlantedBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, bombOpaqueBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.5f}, bombTransparentBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, defuseBlueBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.53f, 0.53f, 1}, defuseRedBrush.put());
	renderTarget.CreateLayer(bombLayer.put());
	renderTarget.CreateLayer(defuseLayer.put());
	
	commonResources.writeFactory->CreateTextFormat(
		commonResources.configuration.fontFamily.c_str(), nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	winrt::com_ptr<IDWriteInlineObject> trimmingSign;
	static const DWRITE_TRIMMING TRIMMING_OPTIONS = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
	textFormat->SetTrimming(&TRIMMING_OPTIONS, trimmingSign.get());
	textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	
	textRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
}

void BombTimerComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const auto &bomb = commonResources.bomb;
	const auto currentState = bomb.bombState;
	if (currentState != bombState) {
		if (bombState == BombData::State::DEFUSING)
			defuseTransition.transition(0);
		bombState = currentState;
		if (bombState == BombData::State::PLANTING || bombState == BombData::State::PLANTED) {
			displayedBombState = bombState;
			bombTransition.transition(1);
		} else if (bombState == BombData::State::DEFUSING) {
			defuseTransition.transition(1);
		} else if (bombState != BombData::State::DETONATING) {
			bombTransition.transition(0);
		}
	}
	
	const float bombTransitionValue = bombTransition.getValue();
	if (bombTransitionValue == 0) return;
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	const bool bombTransiting = bombTransition.transiting();
	const float
		outerLeft = parentSize.width * 0.25f,
		outerRight = parentSize.width * 0.75f,
		innerLeft = outerLeft + 4,
		innerRight = outerRight - 4,
		innerWidth = innerRight - innerLeft,
		gaugeHeight = parentSize.height / 2;
	if (bombTransiting) renderTarget.PushLayer(
		{
			{0, 0, parentSize.width, parentSize.height},
			nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
			bombTransitionValue, nullptr,
			D2D1_LAYER_OPTIONS_NONE
		},
		bombLayer.get()
	);
	const bool planting = displayedBombState == BombData::State::PLANTING;
	const float
		bombY = gaugeHeight * (bombTransitionValue - 1),
		bombInnerBottom = gaugeHeight - 4;
	// The reported bomb time from the game tends to exceed 40" when the bomb has just been planted.
	const int displayedBombTime
		= bombState == BombData::State::DETONATING ? 0 : std::min(40000, bomb.oldBombTimeLeft);
	renderTarget.FillRectangle({outerLeft, bombY, outerRight, gaugeHeight + bombY}, gaugeOuterBrush.get());
	renderTarget.FillRectangle(
		{innerLeft, 4 + bombY, innerRight, bombInnerBottom + bombY},
		planting ? gaugeInnerPlantingBrush.get() : gaugeInnerPlantedBrush.get()
	);
	renderTarget.FillRectangle(
		{
			innerLeft, 4 + bombY,
			innerLeft + innerWidth * (
				planting ? 1 - std::min(3000, displayedBombTime) / 3000.f : displayedBombTime / 40000.f
			),
			bombInnerBottom + bombY
		},
		planting ? bombTransparentBrush.get() : bombOpaqueBrush.get()
	);
	textRenderer->draw(
		Utils::formatTimeAmount(displayedBombTime) + L" | "s + bomb.planterName,
		{outerRight+4, bombY, parentSize.width, bombY + gaugeHeight},
		textBrush
	);

	const float defuseTransitionValue = defuseTransition.getValue();
	if (defuseTransitionValue != 0) {
		const bool shouldApplyDefuseLayer = bombTransiting || defuseTransition.transiting();
		if (shouldApplyDefuseLayer) renderTarget.PushLayer(
			{
				{0, bombTransiting ? bombY : gaugeHeight, parentSize.width, parentSize.height},
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				bombTransiting ? bombTransitionValue : defuseTransitionValue, nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			defuseLayer.get()
		);
		const float
			defuseY = bombTransiting ? bombY : gaugeHeight * (defuseTransition.getValue() - 1),
			defuseInnerTop = gaugeHeight + 4,
			defuseInnerBottom = parentSize.height - 4;
		renderTarget.FillRectangle(
			{outerLeft, gaugeHeight + defuseY, outerRight, gaugeHeight*2 + defuseY}, gaugeOuterBrush.get()
		);
		renderTarget.FillRectangle(
			{innerLeft, defuseInnerTop + defuseY, innerRight, defuseInnerBottom + defuseY},
			gaugeInnerPlantedBrush.get()
		);
		if (bombState == BombData::State::DEFUSING) oldBombTime = displayedBombTime;
		if (bombState != BombData::State::DEFUSED) renderTarget.FillRectangle(
			{
				innerLeft + innerWidth * std::max(0, bomb.bombTimeLeft - bomb.defuseTimeLeft) / 40000,
				defuseInnerTop + defuseY,
				innerLeft + innerWidth * oldBombTime / 40000.f,
				defuseInnerBottom + defuseY
			},
			bomb.defuseTimeLeft > bomb.bombTimeLeft ? defuseRedBrush.get() : defuseBlueBrush.get()
		);
		textRenderer->draw(
			Utils::formatTimeAmount(bomb.defuseTimeLeft) + L" | "s + bomb.defuserName,
			{outerRight+4, gaugeHeight + defuseY, parentSize.width, gaugeHeight*2 + defuseY},
			textBrush
		);
		if (shouldApplyDefuseLayer) renderTarget.PopLayer();
	}
	
	if (bombTransiting) renderTarget.PopLayer();
	
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud