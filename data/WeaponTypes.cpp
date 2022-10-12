#include "pch.h"

#include <array>
#include <string>
#include <unordered_map>

#include "data/WeaponTypes.h"

using namespace std::string_literals;

namespace CsgoHud {

// == WeaponTypes ==

WeaponTypes::WeaponTypes() {
	struct WeaponTypeData {
		std::string name;
		std::wstring displayName;
		WeaponCategory category;
	};
	const std::array<WeaponTypeData, 41> data = {{
		{"ak47"s, L"AK-47"s, WeaponCategory::PRIMARY}, // 0
		{"aug"s, L"AUG"s, WeaponCategory::PRIMARY},
		{"awp"s, L"AWP"s, WeaponCategory::PRIMARY},
		{"bizon"s, L"PP-Bizon"s, WeaponCategory::PRIMARY},
		{"famas"s, L"FAMAS"s, WeaponCategory::PRIMARY},
		{"g3sg1"s, L"G3SG1"s, WeaponCategory::PRIMARY}, // 5
		{"galilar"s, L"Galil AR"s, WeaponCategory::PRIMARY},
		{"m249"s, L"M249"s, WeaponCategory::PRIMARY},
		{"m4a1"s, L"M4A4"s, WeaponCategory::PRIMARY},
		{"m4a1_silencer"s, L"M4A1-S"s, WeaponCategory::PRIMARY},
		{"mac10"s, L"MAC-10"s, WeaponCategory::PRIMARY}, // 10
		{"mag7"s, L"Mag-7"s, WeaponCategory::PRIMARY},
		{"mp5sd"s, L"MP5-SD"s, WeaponCategory::PRIMARY},
		{"mp7"s, L"MP7"s, WeaponCategory::PRIMARY},
		{"mp9"s, L"MP9"s, WeaponCategory::PRIMARY},
		{"negev"s, L"Negev"s, WeaponCategory::PRIMARY}, // 15
		{"nova"s, L"Nova"s, WeaponCategory::PRIMARY},
		{"p90"s, L"P90"s, WeaponCategory::PRIMARY},
		{"sawedoff"s, L"Sawed-off"s, WeaponCategory::PRIMARY},
		{"scar20"s, L"SCAR-20"s, WeaponCategory::PRIMARY},
		{"sg556"s, L"SG 553"s, WeaponCategory::PRIMARY}, // 20
		{"ssg08"s, L"SSG 08"s, WeaponCategory::PRIMARY},
		{"ump45"s, L"UMP-45"s, WeaponCategory::PRIMARY},
		{"xm1014"s, L"XM1014"s, WeaponCategory::PRIMARY},
		
		{"cz75a"s, L"CZ75 auto"s, WeaponCategory::SECONDARY},
		{"deagle"s, L"Desert eagle"s, WeaponCategory::SECONDARY}, // 25
		{"elite"s, L"Dual Berettas"s, WeaponCategory::SECONDARY},
		{"fiveseven"s, L"Five-seveN"s, WeaponCategory::SECONDARY},
		{"glock"s, L"Glock-18"s, WeaponCategory::SECONDARY},
		{"hkp2000"s, L"P2000"s, WeaponCategory::SECONDARY},
		{"p250"s, L"P250"s, WeaponCategory::SECONDARY}, // 30
		{"revolver"s, L"R8 revolver"s, WeaponCategory::SECONDARY},
		{"tec9"s, L"Tec-9"s, WeaponCategory::SECONDARY},
		{"usp_silencer"s, L"USP-S"s, WeaponCategory::SECONDARY},
		
		{"taser"s, L"Zeus x27"s, WeaponCategory::ZEUS},
		
		{"decoy"s, L"Decoy grenade"s, WeaponCategory::GRENADE}, // 35
		{"flashbang"s, L"Stun grenade"s, WeaponCategory::GRENADE},
		{"hegrenade"s, L"Frag grenade"s, WeaponCategory::GRENADE},
		{"incgrenade"s, L"Incendiary grenade"s, WeaponCategory::GRENADE},
		{"molotov"s, L"Molotov cocktail"s, WeaponCategory::GRENADE},
		{"smokegrenade"s, L"Smoke grenade"s, WeaponCategory::GRENADE} // 40
	}};

	for (int i = 0; i != data.size(); ++i) {
		const auto &entry = data[i];
		nameMap.insert({entry.name, i});
		categoryMap[i] = entry.category;
		displayNameMap[i] = entry.displayName;
	}
}

} // namespace CsgoHud