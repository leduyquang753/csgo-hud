#include <array>
#include <optional>
#include <string>

#include "pch.h"

#include "data/AllPlayersData.h"
#include "events/EventBus.h"

using namespace std::string_literals;

namespace CsgoHud {

class PlayerData;
class WeaponTypes;

// == AllPlayersData ==

AllPlayersData::AllPlayersData(const WeaponTypes &weaponTypes, EventBus &eventBus): weaponTypes(weaponTypes) {
	eventBus.listenToDataEvent("allplayers"s, [this](const JSON &json) { receivePlayersData(json); });	
	eventBus.listenToDataEvent("phase_countdowns"s, [this](const JSON &json) { receivePhaseData(json); });	
}

void AllPlayersData::receivePlayersData(const JSON &json) {
	//steamIdMap.clear();
	std::array<bool, 10> playerPresent = {};
	for (const JSON &playerData : json) {
		if (!playerData.contains("observer_slot"s)) continue;
		int slot = playerData["observer_slot"s].get<int>();
		if (slot == 0) slot = 10;
		--slot;
		auto &player = players[slot];
		if (!player) player.emplace();
		player->receiveData(weaponTypes, playerData);
		playerPresent[slot] = true;
		//steamIdMap.insert({std::stoull(playerData["steamid"s].get<std::string>()), slot});
	}
	for (int i = 0; i != 10; ++i) if (!playerPresent[i]) players[i].reset();
}

void AllPlayersData::receivePhaseData(const JSON &json) {
	const std::string currentPhase = json["phase"s].get<std::string>();
	if (phase == currentPhase) return;
	phase = currentPhase;
	if (phase == "live"s) {
		alreadyFreezeTime = false;
	} else if (!alreadyFreezeTime && phase == "freezetime"s) {
		alreadyFreezeTime = true;
		for (auto &player : players) if (player) player->startingMoney = player->money;
	}
}

const std::optional<PlayerData>& AllPlayersData::operator[](const int slot) const {
	return players[slot];
}

const std::optional<PlayerData>& AllPlayersData::operator()(const std::uint64_t steamId) const {
	static const std::optional<PlayerData> NO_PLAYER;
	
	auto iterator = steamIdMap.find(steamId);
	return iterator == steamIdMap.end() ? NO_PLAYER : players[iterator->second];
}

int AllPlayersData::getFirstPlayerIndex() const {
	for (int i = 0; i != 10; ++i) if (players[i]) return i;
	return -1;
}

} // namespace CsgoHud