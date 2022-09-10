#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "pch.h"

#include "events/EventBus.h"

#include "data/AllPlayersData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

class PlayerData;
class WeaponTypes;

// == AllPlayersData ==

AllPlayersData::AllPlayersData(const WeaponTypes &weaponTypes, EventBus &eventBus): weaponTypes(weaponTypes) {
	eventBus.listenToDataEvent("allplayers"s, [this](JSON::dom::object &json) { receivePlayersData(json); });
	eventBus.listenToDataEvent(
		"phase_countdowns"s, [this](JSON::dom::object &json) { receivePhaseData(json); }
	);	
}

void AllPlayersData::receivePlayersData(JSON::dom::object &json) {
	steamIdMap.clear();
	std::array<bool, 10> playerPresent = {};
	for (auto entry : json) {
		auto playerData = entry.value.get_object().value();
		auto slotData = playerData["observer_slot"sv];
		if (slotData.error()) continue;
		int slot = static_cast<int>(slotData.value().get_uint64());
		if (slot == 0) slot = 10;
		--slot;
		auto &player = players[slot];
		if (!player) player.emplace();
		player->receiveData(weaponTypes, playerData);
		playerPresent[slot] = true;
		steamIdMap.insert({std::stoull(std::string(entry.key)), slot});
	}
	for (int i = 0; i != 10; ++i) if (!playerPresent[i]) players[i].reset();
}

void AllPlayersData::receivePhaseData(JSON::dom::object &json) {
	const std::string currentPhase(json["phase"sv].value().get_string().value());
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