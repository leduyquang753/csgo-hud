#ifndef CSGO_HUD_DATA_ROUNDSDATA_H
#define CSGO_HUD_DATA_ROUNDSDATA_H

#include <utility>
#include <vector>

#include "pch.h"

namespace CsgoHud {

class EventBus;

/*
	The storage of played rounds' results and current round's status.
*/
class RoundsData final {
	public:
		enum class WinningCondition { ELIMINATION, TIME, BOMB, DEFUSE };
		enum class Phase { WARMUP, FREEZETIME, LIVE, BOMB, DEFUSE, OVER, PAUSED, TIMEOUT_CT, TIMEOUT_T };
	private:
		int currentRound = 1;
		Phase currentPhase = Phase::WARMUP;
		std::vector<std::pair<bool, WinningCondition>> history;

		void receivePhaseData(JSON::dom::object &json);
		void receiveMapData(JSON::dom::object &json);
	public:
		static const std::array<int, 4> iconMap;
		
		RoundsData(EventBus &eventBus);
		const std::vector<std::pair<bool, WinningCondition>>& getRounds() const;
		int getCurrentRound() const;
		Phase getCurrentPhase() const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ROUNDSDATA_H