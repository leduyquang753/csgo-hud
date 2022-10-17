#include "pch.h"

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

	auto teamNamesData = json["defaultTeamNames"sv].value().get_object().value();
	defaultCtName = Utils::widenString(teamNamesData["counterTerrorists"sv].value().get_string().value());
	defaultTName = Utils::widenString(teamNamesData["terrorists"sv].value().get_string().value());

	auto timingData = json["timings"sv].value().get_object().value();
	timings.freezeTime = static_cast<int>(timingData["freezeTime"sv].value().get_int64().value());
	timings.mainTime = static_cast<int>(timingData["mainTime"sv].value().get_int64().value());
	timings.bombTime = static_cast<int>(timingData["bombTime"sv].value().get_int64().value());
	timings.roundEnd = static_cast<int>(timingData["roundEnd"sv].value().get_int64().value());
	timings.halfTime = static_cast<int>(timingData["halfTime"sv].value().get_int64().value());

	auto formattingData = json["formatting"sv].value().get_object().value();
	formatting.addCurrentClipToTotalAmmo = formattingData["addCurrentClipToTotalAmmo"sv].value().get_bool().value();
	formatting.decimalSeparatorComma = formattingData["decimalSeparatorComma"sv].value().get_bool().value();
	formatting.showSecondsSign = formattingData["showSecondsSign"sv].value().get_bool().value();
	formatting.dollarSignAfter = formattingData["dollarSignAfter"sv].value().get_bool().value();
	formatting.dollarSignWithSpace = formattingData["dollarSignWithSpace"sv].value().get_bool().value();
	formatting.showTenthPlayerAsZero = formattingData["showTenthPlayerAsZero"sv].value().get_bool().value();
	
	keybindings = json["keybindings"sv].value().get_object().value();
}

} // namespace CsgoHud