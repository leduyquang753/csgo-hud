#include "pch.h"

#include <array>
#include <fstream>
#include <string>
#include <unordered_map>

#include "utils/Utils.h"

#include "data/WeaponTypes.h"

using namespace std::string_literals;

namespace CsgoHud {

// == WeaponTypes ==

WeaponTypes::WeaponTypes() {
	struct WeaponTypeData {
		std::string name;
		WeaponCategory category;
	};
	const std::array<WeaponTypeData, 41> data = {{
		{"ak47"s, WeaponCategory::PRIMARY}, // 0
		{"aug"s, WeaponCategory::PRIMARY},
		{"awp"s, WeaponCategory::PRIMARY},
		{"bizon"s, WeaponCategory::PRIMARY},
		{"famas"s, WeaponCategory::PRIMARY},
		{"g3sg1"s, WeaponCategory::PRIMARY}, // 5
		{"galilar"s, WeaponCategory::PRIMARY},
		{"m249"s, WeaponCategory::PRIMARY},
		{"m4a1"s, WeaponCategory::PRIMARY},
		{"m4a1_silencer"s, WeaponCategory::PRIMARY},
		{"mac10"s, WeaponCategory::PRIMARY}, // 10
		{"mag7"s, WeaponCategory::PRIMARY},
		{"mp5sd"s, WeaponCategory::PRIMARY},
		{"mp7"s, WeaponCategory::PRIMARY},
		{"mp9"s, WeaponCategory::PRIMARY},
		{"negev"s, WeaponCategory::PRIMARY}, // 15
		{"nova"s, WeaponCategory::PRIMARY},
		{"p90"s, WeaponCategory::PRIMARY},
		{"sawedoff"s, WeaponCategory::PRIMARY},
		{"scar20"s, WeaponCategory::PRIMARY},
		{"sg556"s, WeaponCategory::PRIMARY}, // 20
		{"ssg08"s, WeaponCategory::PRIMARY},
		{"ump45"s, WeaponCategory::PRIMARY},
		{"xm1014"s, WeaponCategory::PRIMARY},

		{"cz75a"s, WeaponCategory::SECONDARY},
		{"deagle"s, WeaponCategory::SECONDARY}, // 25
		{"elite"s, WeaponCategory::SECONDARY},
		{"fiveseven"s, WeaponCategory::SECONDARY},
		{"glock"s, WeaponCategory::SECONDARY},
		{"hkp2000"s, WeaponCategory::SECONDARY},
		{"p250"s, WeaponCategory::SECONDARY}, // 30
		{"revolver"s, WeaponCategory::SECONDARY},
		{"tec9"s, WeaponCategory::SECONDARY},
		{"usp_silencer"s, WeaponCategory::SECONDARY},

		{"taser"s, WeaponCategory::ZEUS},

		{"decoy"s, WeaponCategory::GRENADE}, // 35
		{"flashbang"s, WeaponCategory::GRENADE},
		{"hegrenade"s, WeaponCategory::GRENADE},
		{"incgrenade"s, WeaponCategory::GRENADE},
		{"molotov"s, WeaponCategory::GRENADE},
		{"smokegrenade"s, WeaponCategory::GRENADE} // 40
	}};

	std::ifstream namesFile("Weapon names.json"s);
	std::string jsonString;
	std::getline(namesFile, jsonString, static_cast<char>(std::char_traits<char>::eof()));
	const auto previousSize = jsonString.size();
	jsonString.append(JSON::SIMDJSON_PADDING, '\0');
	JSON::dom::parser jsonParser;
	JSON::dom::element jsonDocument = jsonParser.parse(jsonString.data(), previousSize);
	JSON::dom::array json = jsonDocument.get_array();

	for (int i = 0; i != data.size(); ++i) {
		const auto &entry = data[i];
		nameMap.insert({entry.name, i});
		categoryMap[i] = entry.category;
		displayNameMap[i] = Utils::widenString(json.at(i).value().get_string().value());
	}
}

} // namespace CsgoHud