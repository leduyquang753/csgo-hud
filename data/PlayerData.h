#ifndef CSGO_HUD_DATA_PLAYERDATA_H
#define CSGO_HUD_DATA_PLAYERDATA_H

#include <array>
#include <optional>
#include <string>

#include "pch.h"

#include "data/GunData.h"

namespace CsgoHud {

class WeaponTypes;

/*
	The data of an in-game player.
*/
class PlayerData final {
	public:
		static const bool TEAM_TERRORISTS = false, TEAM_COUNTER_TERRORISTS = true;
		static const int
			SLOT_PRIMARY_GUN = 1, SLOT_SECONDARY_GUN = 2, SLOT_KNIFE = 3, SLOT_ZEUS = 4,
			SLOT_GRENADE_0 = 5, SLOT_GRENADE_1 = 6, SLOT_GRENADE_2 = 7, SLOT_GRENADE_3 = 8,
			SLOT_C4 = 9;
		
		std::wstring name;
		bool team;

		int health;
		int armor;
		bool hasHelmet;
		int flashAmount;
		int smokeAmount;
		int fireAmount;
		int startingMoney;
		int money;
		int killsThisRound;
		int headshotKillsThisRound;
		int equipmentValue;

		D2D_VECTOR_3F position;
		D2D_VECTOR_3F forward;

		int totalKills;
		int totalAssists;
		int totalDeaths;
		int totalMvps;
		int totalScore;

		std::optional<GunData> primaryGun;
		std::optional<GunData> secondaryGun;
		bool hasZeus;
		std::array<std::optional<int>, 4> grenades;
		bool hasC4OrDefuseKit;

		int activeSlot;

		void receiveData(const WeaponTypes &weaponTypes, JSON::dom::object &json);
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_PLAYERDATA_H