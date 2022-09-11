#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pch.h"

#include "data/IconStorage.h"
#include "events/EventBus.h"

#include "data/RoundsData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == RoundsData ==

const std::array<int, 4> RoundsData::iconMap = {
	IconStorage::INDEX_DEAD, IconStorage::INDEX_TIMER, IconStorage::INDEX_EXPLOSION, IconStorage::INDEX_DEFUSE_KIT
};

RoundsData::RoundsData(EventBus &eventBus) {
	eventBus.listenToDataEvent("phase_countdowns"s, [this](JSON::dom::object &json) { receivePhaseData(json); });
	eventBus.listenToDataEvent("map"s, [this](JSON::dom::object &json) { receiveMapData(json); });
}

void RoundsData::receivePhaseData(JSON::dom::object &json) {
	auto phaseString = json["phase"sv].value().get_string().value();
	switch (phaseString[0]) {
		case 'w':
			currentPhase = Phase::WARMUP;
			break;
		case 'f':
			currentPhase = Phase::FREEZETIME;
			break;
		case 'l':
			currentPhase = Phase::LIVE;
			break;
		case 'b':
			currentPhase = Phase::BOMB;
			break;
		case 'd':
			currentPhase = Phase::DEFUSE;
			break;
		case 'p':
			currentPhase = Phase::PAUSED;
			break;
		case 't'/*imeout_*/:
			currentPhase = phaseString[8] == 'c'/*t*/ ? Phase::TIMEOUT_CT : Phase::TIMEOUT_T;
		default:
			currentPhase = Phase::OVER;
			break;
	}
}

void RoundsData::receiveMapData(JSON::dom::object &json) {
	currentRound = static_cast<int>(json["round"sv].value().get_int64());
	auto historyData = json["round_wins"sv].value().get_object().value();
	history.clear();
	history.reserve(historyData.size());
	for (auto entry : historyData) {
		std::string_view winString = entry.value.get_string();
		const bool winningTeam = winString[0] == 'c'/*t*/;
		WinningCondition condition;
		if (winningTeam) {
			const char conditionFirstChar = winString[7]; // ct_win_
			condition
				= conditionFirstChar == 'e'/*limination*/ ? WinningCondition::ELIMINATION
				: conditionFirstChar == 't'/*ime*/ ? WinningCondition::TIME
				: WinningCondition::DEFUSE;
		} else {
			const char conditionFirstChar = winString[6]; // t_win_
			condition = conditionFirstChar == 'e'/*limination*/
				? WinningCondition::ELIMINATION
				: WinningCondition::BOMB;
		}
		history.emplace_back(winningTeam, condition);
	}
}

const std::vector<std::pair<bool, RoundsData::WinningCondition>>& RoundsData::getRounds() const {
	return history;
}

int RoundsData::getCurrentRound() const {
	return currentPhase == Phase::OVER ? currentRound : currentRound + 1;
}

RoundsData::Phase RoundsData::getCurrentPhase() const {
	return currentPhase;
}

} // namespace CsgoHud