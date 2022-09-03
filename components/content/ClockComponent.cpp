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

static const int FONT_SIZE = 24;
static const float TEXT_OFFSET = -1;

// == ClockComponent ==

ClockComponent::ClockComponent(CommonResources &commonResources): Component(commonResources) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.53f, 0.53f, 1}, textRedBrush.put());
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, backgroundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.2f}, progressWhiteBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.4f, 0.4f, 0.4f}, progressRedBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		FONT_SIZE, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	
	textRenderer.emplace(commonResources, textFormat);

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
	eventBus.listenToDataEvent("phase_countdowns"s, [this](const JSON &json) { receiveData(json); });
}

void ClockComponent::advanceTime(const int timePassed) {
	phaseTimeLeft = std::max(0, phaseTimeLeft - timePassed);
}

void ClockComponent::receiveData(const JSON &json) {
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
		if (!currentPhase.empty()) phaseTimeLeft = timeLeft;
	}
}

int ClockComponent::getPhaseTime() {
	return
		phase == "live"s ? 115000
		: phase == "freezetime"s ? 15000
		: 0;
}

void ClockComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	const float textPosition = (parentSize.height - FONT_SIZE) / 2 + TEXT_OFFSET;
	
	renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());
	if (phase == "paused"s) {
		textRenderer->draw(
			L"Paused"s,
			{0, textPosition, parentSize.width, parentSize.height},
			textWhiteBrush
		);
	} else if (!phase.empty()) {
		const int phaseTime = getPhaseTime();
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
			{0, textPosition, parentSize.width, parentSize.height},
			red ? textRedBrush : textWhiteBrush
		);
	}
	
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud