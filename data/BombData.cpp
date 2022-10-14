#include "pch.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>

#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "data/BombData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == BombData ==

BombData::BombData(CommonResources &commonResources): commonResources(commonResources) {
	auto &eventBus = commonResources.eventBus;
	eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
	eventBus.listenToDataEvent("bomb"s, [this](JSON::dom::object &json) { receiveBombData(json); });
}

void BombData::advanceTime(const int timePassed) {
	if (bombState == State::PLANTED || bombState == State::DEFUSING) {
		if (bombTimeLeft <= timePassed) {
			bombState = State::DETONATING;
			bombTimeLeft = std::max(0, bombTimeLeft + 1000 - timePassed);
		} else {
			bombTimeLeft -= timePassed;
		}
	} else if (bombState == State::EXPLODED) {
		// Used for the explosion animation.
		bombTimeLeft += timePassed;
	} else {
		// Just clamp the time, the game will tell about the state transition.
		bombTimeLeft = std::max(0, bombTimeLeft - timePassed);
	}

	if (bombState == State::PLANTING || bombState == State::PLANTED || bombState == State::DEFUSING) {
		if (!planterFound) {
			const auto &planter = commonResources.players(planterSteamId);
			if (planter) {
				planterFound = true;
				planterName = planter->name;
			}

			if (bombState == State::DEFUSING && !defuserFound) {
				const auto &defuser = commonResources.players(planterSteamId);
				if (defuser) {
					defuserFound = true;
					defuserName = defuser->name;
				}
			}
		}

		oldBombTimeLeft = bombTimeLeft;
		if (bombState == State::DEFUSING) defuseTimeLeft = std::max(0, defuseTimeLeft - timePassed);
	}

}

void BombData::receiveBombData(JSON::dom::object &json) {
	auto stateString = json["state"sv].value().get_string().value();
	bombPosition = Utils::parseVector(json["position"sv].value().get_string());
	State currentState;
	switch (stateString[0]) {
		case 'c':
			currentState = State::CARRIED;
			break;
		case 'd':
			currentState
				= stateString[1] == 'r' ? State::DROPPED
				: stateString[5] == 'i' ? State::DEFUSING
				: State::DEFUSED;
			break;
		case 'e':
			currentState = State::EXPLODED;
			break;
		case 'p':
			currentState = stateString[5] == 'i' ? State::PLANTING : State::PLANTED;
			break;
	}
	// Need to check as there might be a brief moment the payload doesn't contain the field.
	auto countdown = json["countdown"sv];
	const int timeLeft = countdown.error()
		? -1
		: static_cast<int>(std::stod(std::string(json["countdown"sv].value().get_string().value())) * 1000);
	if (currentState == bombState) {
		if (timeLeft != -1) {
			if (currentState == State::PLANTING) {
				if (std::abs(timeLeft - bombTimeLeft) > CommonConstants::DESYNC_THRESHOLD)
					bombTimeLeft = timeLeft;
			} else if (currentState == State::DEFUSING) {
				if (std::abs(timeLeft - defuseTimeLeft) > CommonConstants::DESYNC_THRESHOLD)
					defuseTimeLeft = timeLeft;
			}
		}
	} else if (bombState != State::DETONATING || currentState != State::PLANTED || timeLeft > 0) {
		bombState = currentState;
		if (currentState == State::PLANTING || currentState == State::PLANTED) {
			if (currentState == State::PLANTING) {
				planterSteamId = json["player"sv].value().get_uint64();
				const auto &planter = commonResources.players(planterSteamId);
				planterFound = planter.has_value();
				planterName = planterFound ? planter->name : L"?"s;
			}
			bombTimeLeft = timeLeft;
		} else if (currentState == State::DEFUSING) {
			defuseTimeLeft = timeLeft;
			defuserSteamId = json["player"sv].value().get_uint64();
			const auto &defuser = commonResources.players(defuserSteamId);
			defuserFound = defuser.has_value();
			defuserName = defuserFound ? defuser->name : L"?"s;
		} else if (currentState == State::DEFUSED) {
			bombTimeLeft = 0;
		}
	}
}

} // namespace CsgoHud