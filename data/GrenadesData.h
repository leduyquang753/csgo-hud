#ifndef CSGO_HUD_DATA_GRENADESDATA_H
#define CSGO_HUD_DATA_GRENADESDATA_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "data/RoundsData.h"

namespace CsgoHud {

struct CommonResources;

/*
	Keeps track of grenades thrown out by players.
*/
class GrenadesData final {
	public:
		struct Grenade {
			bool stillAlive;
			D2D1_VECTOR_3F position;
			std::uint64_t throwerId = 0;
			bool throwerFound = false, throwerTeam;
		};
		struct FragGrenade final: Grenade {
			int explodingTime = -1;
		};
		struct SmokeGrenade final: Grenade {
			int smokingTime = 0;
		};
		struct BurningPiece final {
			bool stillAlive;
			D2D1_VECTOR_3F position;
			int burningTime = 0;
		};
		struct BurningArea final {
			bool stillAlive, extinguished = false;
			int burningTime = 0;
			std::unordered_map<std::string, BurningPiece> pieceMap;
		};
		struct ExplodedGrenade final {
			D2D1_VECTOR_3F position;
			int animationTime = 0;
		};
	private:
		CommonResources &commonResources;
		RoundsData::Phase previousPhase = RoundsData::Phase::WARMUP;

		void advanceTime(int timePassed);
		void receiveGrenadesData(JSON::dom::object &json);
	public:
		std::unordered_map<unsigned long, FragGrenade> fragGrenades;
		std::unordered_map<unsigned long, Grenade> stunGrenades;
		std::unordered_map<unsigned long, SmokeGrenade> smokeGrenades;
		std::unordered_map<unsigned long, Grenade> incendiaryGrenades;
		std::unordered_map<unsigned long, Grenade> decoyGrenades;
		std::unordered_map<unsigned long, BurningArea> burningAreas;

		std::vector<ExplodedGrenade> explodedStunGrenades;
		std::vector<ExplodedGrenade> explodedDecoyGrenades;

		GrenadesData(CommonResources &commonResources);
		void purgeExpiredEntities();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_GRENADESDATA_H