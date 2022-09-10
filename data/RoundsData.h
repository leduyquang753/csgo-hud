#ifndef CSGO_HUD_DATA_ROUNDSDATA_H
#define CSGO_HUD_DATA_ROUNDSDATA_H

#include <utility>
#include <vector>

#include "pch.h"

namespace CsgoHud {

class EventBus;

/*
	The storage of played rounds' results.
*/
class RoundsData final {
	public:
		enum class WinningCondition { ELIMINATION, TIME, BOMB, DEFUSE };
	private:
		int currentRound = 1;
		std::vector<std::pair<bool, WinningCondition>> history;

		void receiveMapData(JSON::dom::object &json);
	public:
		static const std::array<int, 4> iconMap;
		
		RoundsData(EventBus &eventBus);
		const std::vector<std::pair<bool, WinningCondition>>& getRounds() const;
		int getCurrentRound() const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ROUNDSDATA_H