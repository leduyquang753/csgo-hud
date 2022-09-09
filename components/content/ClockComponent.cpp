#include "pch.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>

#include "components/base/Component.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/ClockComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == ClockComponent ==

ClockComponent::ClockComponent(CommonResources &commonResources): Component(commonResources) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.53f, 0.53f, 1}, textRedBrush.put());
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, backgroundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.2f}, progressWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.4f, 0.4f, 0.4f}, progressRedBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	
	textRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
	eventBus.listenToDataEvent("phase_countdowns"s, [this](const JSON &json) { receivePhaseData(json); });
	eventBus.listenToDataEvent("map"s, [this](const JSON &json) { receiveMapData(json); });
}

void ClockComponent::advanceTime(const int timePassed) {
	phaseTimeLeft = std::max(0, phaseTimeLeft - timePassed);
}

void ClockComponent::receivePhaseData(const JSON &json) {
	std::string currentPhase = json["phase"s].get<std::string>();
	if (currentPhase == "bomb"s || currentPhase == "defuse"s) currentPhase.clear();
	int timeLeft;
	if (!currentPhase.empty()) {
		const std::string timeString = json["phase_ends_in"s].get<std::string>();
		timeLeft
			= std::stoi(timeString.substr(0, timeString.size()-2)) * 1000
			+ std::stoi(timeString.substr(timeString.size()-1, 1)) * 100;
	}
	if (currentPhase == phase) {
		if (!currentPhase.empty()) {
			if (std::abs(timeLeft - phaseTimeLeft) > CommonConstants::DESYNC_THRESHOLD)
				phaseTimeLeft = timeLeft;
		}
	} else {
		phase = currentPhase;
		phaseTime = getPhaseTime();
		if (!currentPhase.empty()) phaseTimeLeft = timeLeft;
	}
}

void ClockComponent::receiveMapData(const JSON &json) {
	const std::string currentPhase = json["phase"s].get<std::string>();
	if (currentPhase != mapPhase) {
		mapPhase = currentPhase;
		phaseTime = getPhaseTime();
	}
}

int ClockComponent::getPhaseTime() {
	return
		mapPhase == "intermission"s ? 15000
		: phase == "live"s ? 115000
		: phase == "freezetime"s ? 15000
		: phase == "over"s ? 7000
		: 0;
}

void ClockComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	
	renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());
	if (phase == "paused"s) {
		const float
			center = parentSize.width / 2,
			outer = parentSize.height / 4,
			inner = parentSize.height / 12,
			iconTop = parentSize.height / 4,
			iconBottom = parentSize.height * 3 / 4;
		renderTarget.FillRectangle({center-outer, iconTop, center-inner, iconBottom}, textWhiteBrush.get());
		renderTarget.FillRectangle({center+inner, iconTop, center+outer, iconBottom}, textWhiteBrush.get());
	} else if (!phase.empty()) {
		const bool red = phase == "live"s && phaseTimeLeft <= 10000;
		if (phaseTime != 0) {
			renderTarget.FillRectangle(
				{
					0, 0,
					static_cast<float>(std::min(phaseTime, phaseTimeLeft)) * parentSize.width / phaseTime,
					parentSize.height
				},
				red ? progressRedBrush.get() : progressWhiteBrush.get()
			);
		}
		textRenderer->draw(
			Utils::formatTimeAmount(phaseTimeLeft),
			{0, 0, parentSize.width, parentSize.height},
			red ? textRedBrush : textWhiteBrush
		);
	}
	
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud