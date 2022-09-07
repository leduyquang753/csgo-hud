#include <string>
#include <string_view>

#include "pch.h"

#include "data/WeaponTypes.h"
#include "utils/Utils.h"

#include "data/PlayerData.h"

using namespace std::string_literals;

namespace CsgoHud {

static D2D_VECTOR_3F parseVector(std::string_view string) {
	const auto leftComma = string.find(','), rightComma = string.rfind(',');
	return {
		.x = std::stof(std::string(string.substr(0, leftComma))),
		.y = std::stof(std::string(string.substr(leftComma + 2, rightComma - leftComma - 2))),
		.z = std::stof(std::string(string.substr(rightComma + 2, string.size() - rightComma - 2)))
	};
}

// == PlayerData ==

void PlayerData::receiveData(const WeaponTypes &weaponTypes, const JSON &json) {
	name = Utils::widenString(json["name"s].get<std::string>());
	team = json["team"s].get<std::string>()[0] == 'C'/*T*/;

	hasC4OrDefuseKit = false;
	const JSON &state = json["state"s];
	health = state["health"s].get<int>();
	armor = state["armor"s].get<int>();
	hasHelmet = state["helmet"s].get<bool>();
	flashAmount = state["flashed"s].get<int>();
	smokeAmount = state.contains("smoked"s) ? state["smoked"s].get<int>() : 0;
	fireAmount = state["burning"s].get<int>();
	money = state["money"s].get<int>();
	killsThisRound = state["round_kills"s].get<int>();
	headshotKillsThisRound = state["round_killhs"s].get<int>();
	equipmentValue = state["equip_value"s].get<int>();
	if (state.contains("defusekit"s)) hasC4OrDefuseKit = true;

	position = parseVector(json["position"s].get<std::string>());
	forward = parseVector(json["forward"s].get<std::string>());

	const JSON &matchStats = json["match_stats"s];
	totalKills = matchStats["kills"s].get<int>();
	totalAssists = matchStats["assists"s].get<int>();
	totalDeaths = matchStats["deaths"s].get<int>();
	totalMvps = matchStats["mvps"s].get<int>();
	totalScore = matchStats["score"s].get<int>();

	primaryGun.reset();
	secondaryGun.reset();
	hasZeus = false;
	for (int i = 0; i != 4; ++i) grenades[i].reset();
	int nextGrenade = 0;

	const auto &nameMap = weaponTypes.nameMap;
	for (const JSON &weapon : json["weapons"s]) {
		const bool active = weapon["state"s].get<std::string>()[0] == 'a'/*ctive*/;
		if (weapon.contains("type"s) && weapon["type"s].get<std::string>() == "Knife"s) {
			if (active) activeSlot = SLOT_KNIFE;
		} else {
			const std::string name = weapon["name"s].get<std::string>().substr(7); // Skip "weapon_".
			if (name == "c4"s) {
				hasC4OrDefuseKit = true;
				if (active) activeSlot = SLOT_C4;
			} else {
				auto entry = nameMap.find(name);
				if (entry != nameMap.end()) {
					const int type = entry->second;
					switch (weaponTypes.categoryMap[type]) {
						case WeaponCategory::PRIMARY:
							primaryGun = {
								.type = type,
								.roundsInClip = weapon["ammo_clip"s].get<int>(),
								.clipSize = weapon["ammo_clip_max"s].get<int>(),
								.spareRounds = weapon["ammo_reserve"s].get<int>()
							};
							if (active) activeSlot = SLOT_PRIMARY_GUN;
							break;
						case WeaponCategory::SECONDARY:
							secondaryGun = {
								.type = type,
								.roundsInClip = weapon["ammo_clip"s].get<int>(),
								.clipSize = weapon["ammo_clip_max"s].get<int>(),
								.spareRounds = weapon["ammo_reserve"s].get<int>()
							};
							if (active) activeSlot = SLOT_SECONDARY_GUN;
							break;
						case WeaponCategory::ZEUS:
							hasZeus = true;
							if (active) activeSlot = SLOT_ZEUS;
							break;
						case WeaponCategory::GRENADE:
							if (active) activeSlot = SLOT_GRENADE_0 + nextGrenade;
							const int quantity = weapon["ammo_reserve"s].get<int>();
							for (int i = 0; i != quantity; ++i) {
								grenades[nextGrenade] = type;
								++nextGrenade;
							}
							break;
					}
				}
			}
		}
	}
}

} // namespace CsgoHud