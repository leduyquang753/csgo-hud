#include "pch.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "components/base/Component.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/BombTimerComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

static const int FONT_SIZE = 12;
static const float TEXT_OFFSET = -0.5f;

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
	
	auto &writeFactory = *commonResources.writeFactory;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		FONT_SIZE, L"", textFormat.put()
	);
	
	textRenderer.emplace(commonResources, textFormat);

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
	eventBus.listenToDataEvent("bomb"s, [this](const JSON &json) { receiveData(json); });
}

void BombTimerComponent::advanceTime(const int timePassed) {
	if (bombState == "planting"s || bombState == "planted"s || bombState == "defusing"s) {
		bombTimeLeft = std::max(0, bombTimeLeft - timePassed);
		if (bombState == "defusing"s)
			defuseTimeLeft = std::max(0, defuseTimeLeft - timePassed);
	}
}

void BombTimerComponent::receiveData(const JSON &json) {
	#define getTimeLeft() static_cast<int>(std::stod(json["countdown"s].get<std::string>()) * 1000)
	const std::string currentState = json["state"s].get<std::string>();
	if (currentState == bombState) {
		// Need to check as there might be a brief moment the payload doesn't contain the field.
		if (json.contains("countdown"s)) {
			const int timeLeft = getTimeLeft();
			if (currentState == "planting"s || currentState == "planted"s) {
				if (std::abs(timeLeft - bombTimeLeft) > CommonConstants::DESYNC_THRESHOLD)
					bombTimeLeft = timeLeft;
			} else if (currentState == "defusing"s) {
				if (std::abs(timeLeft - defuseTimeLeft) > CommonConstants::DESYNC_THRESHOLD)
					defuseTimeLeft = timeLeft;
			}
		}
	} else {
		if (bombState == "defusing"s) {
			defuseTransition.transition(0);
			oldBombTime = bombTimeLeft;
		}
		bombState = currentState;
		if (currentState == "planting"s || currentState == "planted"s) {
			displayedBombState = currentState;
			if (currentState == "planting"s)
				bombTransition.transition(1);
			bombTimeLeft = getTimeLeft();
		} else if (currentState == "defusing"s) {
			defuseTimeLeft = getTimeLeft();
			defuseTransition.transition(1);
		} else {
			bombTransition.transition(0);
		}
	}
	#undef getTimeLeft
}

void BombTimerComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	
	if (
		bombState == "planting"s || bombState == "planted"s || bombState == "defusing"s
		|| bombTransition.transiting()
	) {
		const float
			outerLeft = parentSize.width * 0.25f,
			outerRight = parentSize.width * 0.75f,
			innerLeft = outerLeft + 4,
			innerRight = outerRight - 4,
			innerWidth = innerRight - innerLeft,
			gaugeHeight = parentSize.height / 2;
		renderTarget.PushLayer(
			{
				{0, 0, parentSize.width, parentSize.height},
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				bombTransition.getValue(), nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			bombLayer.get()
		);
		const bool planting = displayedBombState == "planting"s;
		const float
			bombY = gaugeHeight * (bombTransition.getValue() - 1),
			bombInnerBottom = gaugeHeight - 4;
		// The reported bomb time from the game tends to exceed 40" when the bomb has just been planted.
		const int displayedBombTime = std::min(40000, bombTimeLeft);
		renderTarget.FillRectangle({outerLeft, bombY, outerRight, gaugeHeight + bombY}, gaugeOuterBrush.get());
		renderTarget.FillRectangle(
			{innerLeft, 4 + bombY, innerRight, bombInnerBottom + bombY},
			planting ? gaugeInnerPlantingBrush.get() : gaugeInnerPlantedBrush.get()
		);
		renderTarget.FillRectangle(
			{
				innerLeft, 4 + bombY,
				innerLeft + innerWidth * (planting ? 1 - displayedBombTime / 3000.f : displayedBombTime / 40000.f),
				bombInnerBottom + bombY
			},
			planting ? bombTransparentBrush.get() : bombOpaqueBrush.get()
		);
		textRenderer->draw(
			Utils::formatTimeAmount(displayedBombTime),
			{outerRight+4, (gaugeHeight-FONT_SIZE)/2 + bombY + TEXT_OFFSET, parentSize.width, parentSize.height},
			textBrush
		);

		if (bombState == "defusing"s || defuseTransition.transiting()) {
			const bool planted = bombState == "planted"s || bombState == "defusing"s;
			renderTarget.PushLayer(
				{
					{0, planted ? gaugeHeight : 0.f, parentSize.width, parentSize.height},
					nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
					planted ? defuseTransition.getValue() : bombTransition.getValue(), nullptr,
					D2D1_LAYER_OPTIONS_NONE
				},
				defuseLayer.get()
			);
			const float
				defuseY = planted || defuseTransition.transiting()
					? gaugeHeight * (defuseTransition.getValue() - 1)
					: 0,
				defuseInnerTop = gaugeHeight + 4,
				defuseInnerBottom = parentSize.height - 4;
			renderTarget.FillRectangle(
				{outerLeft, gaugeHeight + defuseY, outerRight, parentSize.height}, gaugeOuterBrush.get()
			);
			renderTarget.FillRectangle(
				{innerLeft, defuseInnerTop + defuseY, innerRight, defuseInnerBottom + defuseY},
				gaugeInnerPlantedBrush.get()
			);
			if (bombState != "defused"s) renderTarget.FillRectangle(
				{
					innerLeft + innerWidth * std::max(0, bombTimeLeft - defuseTimeLeft) / 40000,
					defuseInnerTop + defuseY,
					innerLeft + innerWidth * (bombState == "defusing"s ? bombTimeLeft : oldBombTime) / 40000.f,
					defuseInnerBottom + defuseY
				},
				defuseTimeLeft > bombTimeLeft ? defuseRedBrush.get() : defuseBlueBrush.get()
			);
			textRenderer->draw(
				Utils::formatTimeAmount(defuseTimeLeft),
				{
					outerRight+4, gaugeHeight + (gaugeHeight-FONT_SIZE) / 2 + defuseY + TEXT_OFFSET,
					parentSize.width, parentSize.height
				},
				textBrush
			);
			renderTarget.PopLayer();
		}
		
		renderTarget.PopLayer();
	}
	
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud