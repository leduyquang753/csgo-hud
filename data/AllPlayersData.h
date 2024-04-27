#ifndef CSGO_HUD_DATA_ALLPLAYERSDATA_H
#define CSGO_HUD_DATA_ALLPLAYERSDATA_H

#include <array>
#include <cstdint>
#include <unordered_map>

#include "data/PlayerData.h"

namespace CsgoHud {

struct CommonResources;
class EventBus;

/*
	This class processes and stores all players' data for easy retrieval during rendering.
*/
class AllPlayersData final {
	private:
		CommonResources &commonResources;
		std::array<std::optional<PlayerData>, 10>
			players1, players2,
			*currentPlayers = &players1, *previousPlayers = &players2;
		std::unordered_map<std::uint64_t, int>
			steamIdMap1, steamIdMap2,
			*currentSteamIdMap = &steamIdMap1, *previousSteamIdMap = &steamIdMap2;
		std::string phase;
		bool alreadyFreezeTime = false;
		int firstPlayerIndex = -1;
		int activePlayerIndex = -1;

		void receivePlayerData(JSON::dom::object &json);
		void receivePlayersData(JSON::dom::object &json);
		void receivePhaseData(JSON::dom::object &json);
	public:
		AllPlayersData(CommonResources &commonResources);
		const std::optional<PlayerData>& operator[](int slot) const;
		const std::optional<PlayerData>& operator()(std::uint64_t steamId) const;
		int getFirstPlayerIndex() const;
		int getActivePlayerIndex() const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ALLPLAYERSDATA_H