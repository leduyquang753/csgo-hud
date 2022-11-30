#include "pch.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "data/RoundsData.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "data/GrenadesData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// Utility functions.

template<class MapType> static void unsetStillAlive(MapType &map) {
	for (auto &entry : map) entry.second.stillAlive = false;
}

template<class MapType, class Callback> static void purgeNotAlive(MapType &map, const Callback &callback) {
	auto iterator = map.begin();
	const auto end = map.end();
	while (iterator != end) {
		if (iterator->second.stillAlive) {
			++iterator;
		} else {
			callback(iterator->second);
			iterator = map.erase(iterator);
		}
	}
}

template<class Grenade> static Grenade& getGrenade(
	std::unordered_map<unsigned long, Grenade> &map, const unsigned long id
) {
	auto &grenade = map.emplace(id, Grenade{}).first->second;
	grenade.stillAlive = true;
	return grenade;
}

static void purgeExplodedGrenades(std::vector<GrenadesData::ExplodedGrenade> &grenades, const int lifetime) {
	auto iterator = grenades.begin(), end = grenades.end();
	while (iterator != end) {
		if (iterator->animationTime < lifetime) {
			++iterator;
		} else {
			--end;
			*iterator = std::move(*end);
		}
	}
	grenades.resize(end - grenades.begin());
}

int parseTime(const std::string_view string) {
	return static_cast<int>(std::stod(std::string(string)) * 1000);
}

// == GrenadesData ==

GrenadesData::GrenadesData(CommonResources &commonResources): commonResources(commonResources) {
	commonResources.eventBus.listenToTimeEvent([this](const int timePassed){ advanceTime(timePassed); });
	commonResources.eventBus.listenToDataEvent(
		"grenades"s, [this](JSON::dom::object &json){ receiveGrenadesData(json); }
	);
}

void GrenadesData::advanceTime(const int timePassed) {
	const auto currentPhase = commonResources.rounds.getCurrentPhase();
	if (currentPhase != previousPhase && currentPhase == RoundsData::Phase::FREEZETIME) {
		// Seems like a new round, clear everything.
		fragGrenades.clear();
		stunGrenades.clear();
		smokeGrenades.clear();
		incendiaryGrenades.clear();
		decoyGrenades.clear();
		burningAreas.clear();

		explodedStunGrenades.clear();
		explodedDecoyGrenades.clear();
	} else {
		for (auto &entry : fragGrenades)
			if (entry.second.explodingTime != -1) entry.second.explodingTime += timePassed;
		for (auto &entry : smokeGrenades)
			if (entry.second.smokingTime != 0) entry.second.smokingTime += timePassed;
		{
			auto iterator = burningAreas.begin();
			const auto end = burningAreas.end();
			while (iterator != end) {
				auto &area = iterator->second;
				area.burningTime += timePassed;
				if (area.extinguished && area.burningTime >= 500) {
					iterator = burningAreas.erase(iterator);
				} else {
					for (auto &subEntry : area.pieceMap) subEntry.second.burningTime += timePassed;
					++iterator;
				}
			}
		}
		for (auto &grenade : explodedStunGrenades) grenade.animationTime += timePassed;
		for (auto &grenade : explodedDecoyGrenades) grenade.animationTime += timePassed;
	}
	previousPhase = currentPhase;
}

void GrenadesData::receiveGrenadesData(JSON::dom::object &json) {
	unsetStillAlive(fragGrenades);
	unsetStillAlive(stunGrenades);
	unsetStillAlive(smokeGrenades);
	unsetStillAlive(incendiaryGrenades);
	unsetStillAlive(decoyGrenades);
	unsetStillAlive(burningAreas);

	for (auto entry : json) {
		const unsigned long id = std::stoul(std::string(entry.key));
		auto data = entry.value.get_object().value();
		Grenade *grenadeToResolve = nullptr;
		auto typeString = data["type"sv].value().get_string().value();
		switch (typeString[0]) {
			case 'f':
				switch (typeString[1]) {
					case 'i'/*rebomb*/:
						grenadeToResolve = &getGrenade(incendiaryGrenades, id);
						break;
					case 'l'/*ashbang*/:
						grenadeToResolve = &getGrenade(stunGrenades, id);
						break;
					case 'r'/*ag*/: {
						auto &grenade = getGrenade(fragGrenades, id);
						if (grenade.explodingTime == -1) {
							const auto velocity
								= Utils::parseVector(data["velocity"sv].value().get_string().value());
							if (velocity.x == 0 && velocity.y == 0 && velocity.z == 0) grenade.explodingTime = 0;
						}
						grenadeToResolve = &grenade;
						break;
					}
				}
				break;
			case 's'/*moke*/: {
				auto &grenade = getGrenade(smokeGrenades, id);
				const int smokingTime = parseTime(data["effecttime"sv].value().get_string().value());
				if (smokingTime != 0 && (
					grenade.smokingTime == 0
					|| std::abs(smokingTime - grenade.smokingTime) >= CommonConstants::DESYNC_THRESHOLD
				)) grenade.smokingTime = smokingTime;
				grenadeToResolve = &grenade;
				break;
			}
			case 'd'/*ecoy*/:
				grenadeToResolve = &getGrenade(decoyGrenades, id);
				break;
			case 'i'/*nferno*/: {
				auto &area = burningAreas.emplace(id, BurningArea{}).first->second;
				area.stillAlive = true;
				const int burningTime = parseTime(data["lifetime"sv].value().get_string().value());
				if (burningTime != 0 && (
					area.burningTime == 0
					|| std::abs(burningTime - area.burningTime) >= CommonConstants::DESYNC_THRESHOLD
				)) area.burningTime = burningTime;
				auto &pieces = area.pieceMap;
				for (auto &entry : pieces) entry.second.stillAlive = false;
				for (auto entry : data["flames"sv].value().get_object().value()) {
					const std::string subId(entry.key);
					const auto subIterator = pieces.find(subId);
					if (subIterator == pieces.end()) {
						pieces.emplace(
							subId, 
							BurningPiece{true, Utils::parseVector(entry.value.get_string().value()), 0}
						);
					} else {
						auto &piece = subIterator->second;
						piece.stillAlive = true;
						piece.position = Utils::parseVector(entry.value.get_string().value());
					}
				}
				auto pieceIterator = pieces.begin();
				const auto piecesEnd = pieces.end();
				while (pieceIterator != piecesEnd) {
					if (pieceIterator->second.stillAlive) ++pieceIterator;
					else pieceIterator = pieces.erase(pieceIterator);
				}
				break;
			}
		}
		if (grenadeToResolve != nullptr) {
			grenadeToResolve->position = Utils::parseVector(data["position"sv].value().get_string().value());
			if (grenadeToResolve->throwerId == 0) {
				grenadeToResolve->throwerId
					= std::stoull(std::string(data["owner"sv].value().get_string().value()));
			}
			if (!grenadeToResolve->throwerFound) {
				const auto &thrower = commonResources.players(grenadeToResolve->throwerId);
				if (thrower) {
					grenadeToResolve->throwerFound = true;
					grenadeToResolve->throwerTeam = thrower->team;
				}
			}
		}
	}

	purgeNotAlive(fragGrenades, [this](const Grenade &grenade){});
	purgeNotAlive(stunGrenades, [this](const Grenade &grenade){
		explodedStunGrenades.push_back({grenade.position});
	});
	purgeNotAlive(smokeGrenades, [](const SmokeGrenade &grenade){});
	purgeNotAlive(incendiaryGrenades, [](const Grenade &grenade){});
	purgeNotAlive(decoyGrenades, [this](const Grenade &grenade){
		explodedDecoyGrenades.push_back({grenade.position});
	});
	{
		// Mark disappeared burning areas as extinguished for when a smoke grenade gets in their place.
		auto iterator = burningAreas.begin();
		const auto end = burningAreas.end();
		while (iterator != end) {
			auto &area = iterator->second;
			if (area.stillAlive || area.extinguished) {
				++iterator;
			} else if (area.burningTime < 7000) {
				area.burningTime = (std::max(6000, area.burningTime) - 6000) / 2;
				area.extinguished = true;
				++iterator;
			} else {
				iterator = burningAreas.erase(iterator);
			}
		}
	}
}

void GrenadesData::purgeExpiredEntities() {
	purgeExplodedGrenades(explodedStunGrenades, 250);
	purgeExplodedGrenades(explodedDecoyGrenades, 4000);
}

} // namespace CsgoHud