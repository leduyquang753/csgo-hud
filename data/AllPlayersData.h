#ifndef CSGO_HUD_DATA_ALLPLAYERSDATA_H
#define CSGO_HUD_DATA_ALLPLAYERSDATA_H

#include <array>
#include <cstdint>
#include <unordered_map>

#include "pch.h"

#include "data/PlayerData.h"

namespace CsgoHud {

class EventBus;
class WeaponTypes;

/*
	This class processes and stores all players' data for easy retrieval during rendering.
*/
class AllPlayersData final {
	private:
		const WeaponTypes &weaponTypes;
		std::array<std::optional<PlayerData>, 10> players;
		std::unordered_map<std::uint64_t, int> steamIdMap;

		void receiveData(const JSON &json);
	public:
		AllPlayersData(const WeaponTypes &weaponTypes, EventBus &eventBus);
		const std::optional<PlayerData>& operator[](int slot) const;
		const std::optional<PlayerData>& operator()(std::uint64_t steamId) const;
		int getFirstPlayerIndex() const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ALLPLAYERSDATA_H