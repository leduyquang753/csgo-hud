#ifndef CSGO_HUD_DATA_WEAPONTYPES_H
#define CSGO_HUD_DATA_WEAPONTYPES_H

#include <array>
#include <string>
#include <unordered_map>

namespace CsgoHud {

enum class WeaponCategory {
	PRIMARY, SECONDARY, ZEUS, GRENADE
};

class WeaponTypes final {
	public:
		std::unordered_map<std::string, int> nameMap;
		std::array<WeaponCategory, 41> categoryMap;
		std::array<std::wstring, 41> displayNameMap;

		WeaponTypes();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_WEAPONTYPES_H