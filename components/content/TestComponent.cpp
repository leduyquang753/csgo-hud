#include "pch.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "components/base/Component.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "utils/Utils.h"

#include "components/content/TestComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == TestComponent ==

TestComponent::TestComponent(CommonResources &commonResources):
	Component(commonResources),
	bombPosition(
		commonResources,
		std::move(std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		)),
		300, -12
	),
	bombOpacity(
		commonResources,
		std::move(std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		)),
		300, 0
	),
	defusePosition(
		commonResources,
		std::move(std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		)),
		300, -12
	),
	defuseOpacity(
		commonResources,
		std::move(std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		)),
		300, 0
	)
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.53f, 0.53f, 1}, textRedBrush.put());
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, timeBlackBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.2f}, timeWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.4f, 0.4f, 0.4f}, timeRedBrush.put());
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
		24, L"", timeTextFormat.put()
	);
	timeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", bombTextFormat.put()
	);

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
	eventBus.listenToDataEvent("bomb"s, [this](const JSON &json) { receiveBombData(json); });
	eventBus.listenToDataEvent("phase_countdowns"s, [this](const JSON &json) { receivePhaseData(json); });
}

void TestComponent::advanceTime(const int timePassed) {
	if (bombState == "planting"s || bombState == "planted"s || bombState == "defusing"s) {
		bombTimeLeft = std::max(0, bombTimeLeft - timePassed);
		if (bombState == "defusing"s)
			defuseTimeLeft = std::max(0, defuseTimeLeft - timePassed);
	}
	phaseTimeLeft = std::max(0, phaseTimeLeft - timePassed);
}

static const int DESYNC_THRESHOLD = 200;

void TestComponent::receiveBombData(const JSON &json) {
	const std::string currentState = json["state"s].get<std::string>();
	if (currentState == bombState) {
		if (json.contains("countdown"s)) {
			const int timeLeft = static_cast<int>(std::stod(json["countdown"s].get<std::string>()) * 1000);
			if (currentState == "planting"s || currentState == "planted"s) {
				if (std::abs(timeLeft - bombTimeLeft) > DESYNC_THRESHOLD)
					bombTimeLeft = timeLeft;
			} else if (currentState == "defusing"s) {
				if (std::abs(timeLeft - defuseTimeLeft) > DESYNC_THRESHOLD)
					defuseTimeLeft = timeLeft;
			}
		}
	} else {
		if (bombState == "defusing"s) {
			defusePosition.transition(-12);
			defuseOpacity.transition(0);
			oldBombTime = bombTimeLeft;
		}
		bombState = currentState;
		if (currentState == "planting"s || currentState == "planted"s) {
			oldBombState = currentState;
			if (currentState == "planting"s) {
				bombPosition.transition(0);
				bombOpacity.transition(1);
			}
			bombTimeLeft = static_cast<int>(std::stod(json["countdown"s].get<std::string>()) * 1000);
		} else if (currentState == "defusing"s) {
			defuseTimeLeft = static_cast<int>(std::stod(json["countdown"s].get<std::string>()) * 1000);
			defusePosition.transition(0);
			defuseOpacity.transition(1);
		} else {
			bombPosition.transition(-12);
			bombOpacity.transition(0);
		}
	}
}

void TestComponent::receivePhaseData(const JSON &json) {
	std::string temp = json["phase"s].get<std::string>();
	if (temp == "bomb"s || temp == "defuse"s) temp.clear();
	if (temp == phase) {
		if (!temp.empty()) {
			temp = json["phase_ends_in"s].get<std::string>();
			const int timeLeft
				= std::stoi(temp.substr(0, temp.size()-2)) * 1000
				+ std::stoi(temp.substr(temp.size()-1, 1)) * 100;
			if (std::abs(timeLeft - phaseTimeLeft) > DESYNC_THRESHOLD)
				phaseTimeLeft = timeLeft;
		}
	} else {
		phase = temp;
		if (!temp.empty()) {
			temp = json["phase_ends_in"s].get<std::string>();
			phaseTimeLeft
				= std::stoi(temp.substr(0, temp.size()-2)) * 1000
				+ std::stoi(temp.substr(temp.size()-1, 1)) * 100;
		}
	}
}

int TestComponent::getPhaseTime() {
	return
		phase == "live"s ? 115000
		: phase == "freezetime"s ? 15000
		: 0;
}

void TestComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	std::wstring wtemp;

	const float middle = parentSize.width / 2;

	// Clock.
	renderTarget.FillRectangle({middle-40, 0, middle+40, 28}, timeBlackBrush.get());
	if (
		phase == "live"s || phase == "freezetime"s || phase == "over"s
		|| phase == "timeout_ct"s || phase == "timeout_t"s
	) {
		const int phaseTime = getPhaseTime();
		const bool red = phase == "live"s && phaseTimeLeft <= 10000;
		if (phaseTime != 0) {
			renderTarget.FillRectangle(
				{middle-40, 0, middle-40 + static_cast<float>(phaseTimeLeft) * 80 / phaseTime, 28},
				red ? timeRedBrush.get() : timeWhiteBrush.get()
			);
		}
		wtemp = Utils::formatTimeAmount(phaseTimeLeft);
		renderTarget.DrawText(
			wtemp.c_str(), static_cast<UINT32>(wtemp.size()),
			timeTextFormat.get(),
			{middle-40, 0, middle+40, 28},
			red ? textRedBrush.get() : textWhiteBrush.get(),
			D2D1_DRAW_TEXT_OPTIONS_NONE,
			DWRITE_MEASURING_MODE_NATURAL
		);
	}

	if (
		bombState == "planting"s || bombState == "planted"s || bombState == "defusing"s
		|| bombPosition.transiting()
	) {
		const float
			outerLeft = parentSize.width * 0.25f,
			outerRight = parentSize.width * 0.75f,
			innerLeft = outerLeft + 4,
			innerRight = outerRight - 4,
			innerWidth = innerRight - innerLeft;
		renderTarget.PushLayer(
			{
				{0, 28, parentSize.width, parentSize.height},
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				bombOpacity.getValue(), nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			bombLayer.get()
		);
		const bool planting = oldBombState == "planting"s;
		const float bombY = bombPosition.getValue();
		// The reported bomb time from the game tends to exceed 40" when the bomb has just been planted.
		const int displayedBombTime = std::min(40000, bombTimeLeft);
		renderTarget.FillRectangle({outerLeft, 28 + bombY, outerRight, 40 + bombY}, gaugeOuterBrush.get());
		renderTarget.FillRectangle(
			{innerLeft, 32 + bombY, innerRight, 36 + bombY},
			planting ? gaugeInnerPlantingBrush.get() : gaugeInnerPlantedBrush.get()
		);
		renderTarget.FillRectangle(
			{
				innerLeft, 32 + bombY,
				innerLeft + innerWidth * (planting ? 1 - bombTimeLeft / 3000.f : bombTimeLeft / 40000.f),
				36 + bombY
			},
			planting ? bombTransparentBrush.get() : bombOpaqueBrush.get()
		);
		wtemp = Utils::formatTimeAmount(bombTimeLeft);
		renderTarget.DrawText(
			wtemp.c_str(), static_cast<UINT32>(wtemp.size()),
			bombTextFormat.get(),
			{outerRight+4, 28+bombY, parentSize.width, parentSize.height},
			textWhiteBrush.get(),
			D2D1_DRAW_TEXT_OPTIONS_NONE,
			DWRITE_MEASURING_MODE_NATURAL
		);

		if (bombState == "defusing"s || defusePosition.transiting()) {
			const bool planted = bombState == "planted"s || bombState == "defusing"s;
			renderTarget.PushLayer(
				{
					{0, planted ? 40.f : 0.f, parentSize.width, parentSize.height},
					nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
					planted ? defuseOpacity.getValue() : bombOpacity.getValue(), nullptr,
					D2D1_LAYER_OPTIONS_NONE
				},
				defuseLayer.get()
			);
			const float defuseY = planted || defusePosition.transiting() ? defusePosition.getValue() : 0;
			renderTarget.FillRectangle({outerLeft, 40 + defuseY, outerRight, 52 + defuseY}, gaugeOuterBrush.get());
			renderTarget.FillRectangle(
				{innerLeft, 44 + defuseY, innerRight, 48 + defuseY},
				gaugeInnerPlantedBrush.get()
			);
			renderTarget.FillRectangle(
				{
					innerLeft + innerWidth * std::max(0, bombTimeLeft - defuseTimeLeft) / 40000,
					44 + defuseY,
					innerLeft + innerWidth * (bombState == "defusing"s ? bombTimeLeft : oldBombTime) / 40000.f,
					48 + defuseY
				},
				defuseTimeLeft > bombTimeLeft ? defuseRedBrush.get() : defuseBlueBrush.get()
			);
			wtemp = Utils::formatTimeAmount(defuseTimeLeft);
			renderTarget.DrawText(
				wtemp.c_str(), static_cast<UINT32>(wtemp.size()),
				bombTextFormat.get(),
				{outerRight+4, 40+defuseY, parentSize.width, parentSize.height},
				textWhiteBrush.get(),
				D2D1_DRAW_TEXT_OPTIONS_NONE,
				DWRITE_MEASURING_MODE_NATURAL
			);
			renderTarget.PopLayer();
		}
		
		renderTarget.PopLayer();
	}
	
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud