#include "pch.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "components/base/Component.h"
#include "data/BombData.h"
#include "data/IconStorage.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/ClockComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

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
	const auto fontFamily = commonResources.configuration.fontFamily.c_str();
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	timeTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, L"", textFormat.put()
	);
	bombsiteNameRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
	eventBus.listenToDataEvent(
		"phase_countdowns"s, [this](JSON::dom::object &json) { receivePhaseData(json); }
	);
	eventBus.listenToDataEvent("map"s, [this](JSON::dom::object &json) { receiveMapData(json); });
}

void ClockComponent::advanceTime(const int timePassed) {
	phaseTimeLeft = std::max(0, phaseTimeLeft - timePassed);
}

void ClockComponent::receivePhaseData(JSON::dom::object &json) {
	std::string currentPhase(json["phase"sv].value().get_string().value());
	int timeLeft;
	if (!currentPhase.empty()) {
		const std::string timeString(json["phase_ends_in"sv].value().get_string().value());
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

void ClockComponent::receiveMapData(JSON::dom::object &json) {
	const std::string currentPhase(json["phase"sv].value().get_string().value());
	if (currentPhase != mapPhase) {
		mapPhase = currentPhase;
		phaseTime = getPhaseTime();
	}
}

int ClockComponent::getPhaseTime() {
	const auto &timings = commonResources.configuration.timings;
	return
		mapPhase == "intermission"s ? timings.halfTime
		: phase == "live"s ? timings.mainTime
		: phase == "freezetime"s ? timings.freezeTime
		: phase == "over"s ? timings.roundEnd
		: 0;
}

void ClockComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const auto &map = commonResources.map;
	const auto &bomb = commonResources.bomb;
	const bool currentBombPlanted
		= bomb.bombState == BombData::State::PLANTED
		|| bomb.bombState == BombData::State::DEFUSING
		|| bomb.bombState == BombData::State::DETONATING;
	if (currentBombPlanted != bombPlanted) {
		bombPlanted = currentBombPlanted;
		if (bombPlanted && map.mapAvailable) bombsiteA
			= map.getDistinguishingAxisValue(bomb.bombPosition) > map.bombsiteDistinguishingValue
			== map.bombsiteAToPositiveSide;
	}
	
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
	} else if (bombPlanted && phase != "over"s) {
		const float center = parentSize.width / 2;
		
		const auto &icon = commonResources.icons[IconStorage::INDEX_C4];
		const float iconWidth
			= icon.width * parentSize.height / CommonConstants::ICON_HEIGHT * 3 / 4;
		renderTarget.DrawBitmap(
			commonResources.icons.getBitmap(),
			map.mapAvailable
				? D2D1_RECT_F{center - iconWidth - 4, parentSize.height/8, center - 4, parentSize.height*7/8}
				: D2D1_RECT_F{
					center - iconWidth/2, parentSize.height/8, center + iconWidth/2, parentSize.height*7/8
				},
			1, D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, icon.floatBounds, nullptr
		);

		if (map.mapAvailable) bombsiteNameRenderer->draw(
			bombsiteA ? L"A"sv : L"B"sv, {center + 4, 0, parentSize.width, parentSize.height}, textWhiteBrush
		);
	} else {
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
		timeTextRenderer->draw(
			Utils::formatTimeAmount(phaseTimeLeft, commonResources.configuration),
			{0, 0, parentSize.width, parentSize.height},
			red ? textRedBrush : textWhiteBrush
		);
	}
	
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud