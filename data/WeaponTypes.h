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
		static constexpr int
			ID_DECOY_GRENADE = 35,
			ID_STUN_GRENADE = 36,
			ID_FRAG_GRENADE = 37,
			ID_INCENDIARY_GRENADE = 38,
			ID_MOLOTOV_COCKTAIL = 39,
			ID_SMOKE_GRENADE = 40;

		std::unordered_map<std::string, int> nameMap;
		std::array<WeaponCategory, 41> categoryMap;
		std::array<std::wstring, 41> displayNameMap;

		WeaponTypes();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_WEAPONTYPES_H