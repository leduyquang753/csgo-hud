#ifndef CSGO_HUD_DATA_BOMBDATA_H
#define CSGO_HUD_DATA_BOMBDATA_H

#include <cstdint>
#include <string>

#include "pch.h"

namespace CsgoHud {

struct CommonResources;

/*
	Stores current state of the bomb.
*/
class BombData final {
	private:
		CommonResources &commonResources;
	public:
		enum class State { DROPPED, CARRIED, PLANTING, PLANTED, DEFUSING, DEFUSED, DETONATING, EXPLODED };

		State bombState = State::DROPPED;
		D2D1_VECTOR_3F bombPosition;

		std::uint64_t planterSteamId, defuserSteamId;
		std::wstring planterName, defuserName;
		int bombTimeLeft, defuseTimeLeft;

		BombData(CommonResources &commonResources);
		void advanceTime(const int timePassed);
		void receiveBombData(JSON::dom::object &json);
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_BOMBDATA_H