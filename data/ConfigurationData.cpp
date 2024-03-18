#include "pch.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <string_view>

#include "utils/Utils.h"

#include "data/ConfigurationData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == ConfigurationData ==

ConfigurationData::ConfigurationData() {
	std::ifstream configurationFile("Configuration.json"s);
	std::string jsonString;
	std::getline(configurationFile, jsonString, static_cast<char>(std::char_traits<char>::eof()));
	const auto previousSize = jsonString.size();
	jsonString.append(JSON::SIMDJSON_PADDING, '\0');
	JSON::dom::element jsonDocument = jsonParser.parse(jsonString.data(), previousSize);
	JSON::dom::object json = jsonDocument.get_object();

	httpServerPort = std::clamp(
		static_cast<int>(json["httpServerPort"sv].value().get_uint64().value()), 0, 65535
	);
	auto resolutionData = json["resolution"sv].value().get_array().value();
	windowWidth = static_cast<int>(resolutionData.at(0).value().get_uint64().value());
	windowHeight = static_cast<int>(resolutionData.at(1).value().get_uint64().value());
	auto scalingData = json["scaling"sv].value().get_array().value();
	hudScaling = static_cast<float>(
		scalingData.at(0).value().get_double().value() / scalingData.at(1).value().get_double().value()
	);
	hudWidth = windowWidth / hudScaling;
	hudHeight = windowHeight / hudScaling;
	startingPositionCenter = json["startingPositionCenter"sv].value().get_bool().value();
	auto offsetData = json["offset"sv].value().get_array().value();
	windowOffsetX = static_cast<int>(offsetData.at(0).value().get_int64().value());
	windowOffsetY = static_cast<int>(offsetData.at(1).value().get_int64().value());
	fontFamily = Utils::widenString(json["fontFamily"sv].value().get_string().value());
	fontOffsetRatio = static_cast<float>(json["fontOffsetRatio"sv].value().get_double().value());
	fontLineHeightRatio = static_cast<float>(json["fontLineHeightRatio"sv].value().get_double().value());

	auto teamNamesData = json["defaultTeamNames"sv].value().get_object().value();
	defaultCtName = Utils::widenString(teamNamesData["counterTerrorists"sv].value().get_string().value());
	defaultTName = Utils::widenString(teamNamesData["terrorists"sv].value().get_string().value());

	auto timingData = json["timings"sv].value().get_object().value();
	timings.freezeTime = static_cast<int>(timingData["freezeTime"sv].value().get_int64().value());
	timings.mainTime = static_cast<int>(timingData["mainTime"sv].value().get_int64().value());
	timings.bombTime = static_cast<int>(timingData["bombTime"sv].value().get_int64().value());
	timings.roundEnd = static_cast<int>(timingData["roundEnd"sv].value().get_int64().value());
	timings.halfTime = static_cast<int>(timingData["halfTime"sv].value().get_int64().value());
	timings.timeout = static_cast<int>(timingData["timeout"sv].value().get_int64().value());

	auto formattingData = json["formatting"sv].value().get_object().value();
	formatting.addCurrentClipToTotalAmmo = formattingData["addCurrentClipToTotalAmmo"sv].value().get_bool().value();
	formatting.decimalSeparatorComma = formattingData["decimalSeparatorComma"sv].value().get_bool().value();
	formatting.showSecondsSign = formattingData["showSecondsSign"sv].value().get_bool().value();
	formatting.dollarSignAfter = formattingData["dollarSignAfter"sv].value().get_bool().value();
	formatting.dollarSignWithSpace = formattingData["dollarSignWithSpace"sv].value().get_bool().value();
	formatting.showTenthPlayerAsZero = formattingData["showTenthPlayerAsZero"sv].value().get_bool().value();

	auto colorsData = json["colors"sv].value().get_object().value();
	auto parseColor = [&colorsData](std::string_view key, D2D1_COLOR_F &color) {
		auto colorData = colorsData[key].value().get_array().value();
		color.r = static_cast<float>(colorData.at(0).value().get_double().value());
		color.g = static_cast<float>(colorData.at(1).value().get_double().value());
		color.b = static_cast<float>(colorData.at(2).value().get_double().value());
		color.a = static_cast<float>(colorData.at(3).value().get_double().value());
	};
	parseColor("ctPrimary"sv, colors.ctPrimary);
	parseColor("ctSecondary"sv, colors.ctSecondary);
	parseColor("tPrimary"sv, colors.tPrimary);
	parseColor("tSecondary"sv, colors.tSecondary);
	parseColor("ctSmoke"sv, colors.ctSmoke);
	parseColor("tSmoke"sv, colors.tSmoke);
	parseColor("damage"sv, colors.damage);
	parseColor("notEnoughTimeToDefuse"sv, colors.notEnoughTimeToDefuse);
	
	keybindings = json["keybindings"sv].value().get_object().value();
}

} // namespace CsgoHud