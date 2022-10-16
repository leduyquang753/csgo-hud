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
	fontFamily = Utils::widenString(json["fontFamily"sv].value().get_string().value());
	keybindings = json["keybindings"sv].value().get_object().value();
}

} // namespace CsgoHud