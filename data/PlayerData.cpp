#include <string>
#include <string_view>

#include "pch.h"

#include "resources/CommonResources.h"
#include "utils/Utils.h"

#include "data/PlayerData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == PlayerData ==

void PlayerData::receiveData(CommonResources &commonResources, JSON::dom::object &json) {
	name = Utils::widenString(json["name"sv].value().get_string());
	team = json["team"sv].value().get_string().value()[0] == 'C'/*T*/;

	auto getInt = [](JSON::dom::object &json, std::string_view key) {
		return static_cast<int>(json[key].value().get_int64());
	};

	hasC4OrDefuseKit = false;
	auto state = json["state"sv].value().get_object().value();
	health = getInt(state, "health"sv);
	armor = getInt(state, "armor"sv);
	hasHelmet = state["helmet"sv].value().get_bool();
	flashAmount = getInt(state, "flashed"sv);
	auto smokeData = state["smoked"sv];
	smokeAmount = smokeData.error() ? 0 : static_cast<int>(smokeData.value().get_int64());
	fireAmount = getInt(state, "burning"sv);
	money = getInt(state, "money"sv);
	killsThisRound = getInt(state, "round_kills"sv);
	headshotKillsThisRound = getInt(state, "round_killhs"sv);
	equipmentValue = getInt(state, "equip_value"sv);
	if (!state["defusekit"sv].error()) hasC4OrDefuseKit = true;

	position = Utils::parseVector(json["position"sv].value().get_string());
	forward = Utils::parseVector(json["forward"sv].value().get_string());

	auto matchStats = json["match_stats"sv].value().get_object().value();
	totalKills = getInt(matchStats, "kills"sv);
	totalAssists = getInt(matchStats, "assists"sv);
	totalDeaths = getInt(matchStats, "deaths"sv);
	totalMvps = getInt(matchStats, "mvps"sv);
	totalScore = getInt(matchStats, "score"sv);

	primaryGun.reset();
	secondaryGun.reset();
	hasZeus = false;
	for (int i = 0; i != 4; ++i) grenades[i].reset();
	int nextGrenade = 0;

	activeSlot = SLOT_NONE;
	const auto &weaponTypes = commonResources.weaponTypes;
	const auto &nameMap = weaponTypes.nameMap;
	for (auto entry : json["weapons"sv].get_object().value()) {
		auto weapon = entry.value.get_object().value();
		const bool active = weapon["state"sv].value().get_string().value()[0] == 'a'/*ctive*/;
		auto typeData = weapon["type"sv];
		if (!typeData.error() && typeData.value().get_string() == "Knife"sv) {
			if (active) activeSlot = SLOT_KNIFE;
		} else {
			const std::string name(weapon["name"sv].value().get_string().value().substr(7)); // Skip "weapon_".
			if (name == "c4"s) {
				hasC4OrDefuseKit = true;
				if (active) activeSlot = SLOT_C4;
			} else {
				auto entry = nameMap.find(name);
				if (entry != nameMap.end()) {
					const int type = entry->second;
					switch (weaponTypes.categoryMap[type]) {
						case WeaponCategory::PRIMARY: {
							auto spareRoundsData = weapon["ammo_reserve"sv];
							primaryGun = {
								.type = type,
								.roundsInClip = getInt(weapon, "ammo_clip"sv),
								.clipSize = getInt(weapon, "ammo_clip_max"sv),
								.spareRounds = spareRoundsData.error()
									? 0
									: static_cast<int>(spareRoundsData.value().get_int64())
							};
							if (active) activeSlot = SLOT_PRIMARY_GUN;
							break;
						}
						case WeaponCategory::SECONDARY: {
							auto spareRoundsData = weapon["ammo_reserve"sv];
							secondaryGun = {
								.type = type,
								.roundsInClip = getInt(weapon, "ammo_clip"sv),
								.clipSize = getInt(weapon, "ammo_clip_max"sv),
								.spareRounds = spareRoundsData.error()
									? 0
									: static_cast<int>(spareRoundsData.value().get_int64())
							};
							if (active) activeSlot = SLOT_SECONDARY_GUN;
							break;
						}
						case WeaponCategory::ZEUS: {
							hasZeus = true;
							if (active) activeSlot = SLOT_ZEUS;
							break;
						}
						case WeaponCategory::GRENADE: {
							if (active) activeSlot = SLOT_GRENADE_0 + nextGrenade;
							const int quantity = getInt(weapon, "ammo_reserve"sv);
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
}

} // namespace CsgoHud