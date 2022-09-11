#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pch.h"

#include "components/base/BagComponent.h"
#include "components/base/Component.h"
#include "components/base/SizedComponent.h"
#include "components/base/StackComponent.h"
#include "components/content/BombTimerComponent.h"
#include "components/content/ClockComponent.h"
#include "data/IconStorage.h"
#include "data/RoundsData.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "text/NormalTextRenderer.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/TopBarComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == TopBarComponent ==

TopBarComponent::TopBarComponent(CommonResources &commonResources):
	Component(commonResources),
	winLoseTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	),
	matchPointTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	),
	timeoutTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	)
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, backgroundBlackBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 0.5f}, ctNameBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctScoreBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.5f}, tNameBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tScoreBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.5f, 1, 0.5f, 1}, moneyGainBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, L"", textFormat.put()
	);
	winrt::com_ptr<IDWriteInlineObject> trimmingSign;
	static const DWRITE_TRIMMING TRIMMING_OPTIONS = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
	textFormat->SetTrimming(&TRIMMING_OPTIONS, trimmingSign.get());
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	nameTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	scoreTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		24, L"", textFormat.put()
	);
	winLoseTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		18, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	moneyGainTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	auto stack = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_HORIZONTAL, StackComponent::AXIS_INCREASE, 0.f, StackComponent::MODE_PIXELS
	);

	auto makeNameStack = [this, &commonResources, &stack](const bool side) {
		auto innerStack = std::make_unique<StackComponent>(
			commonResources,
			StackComponent::AXIS_VERTICAL, StackComponent::AXIS_INCREASE, 0.f, StackComponent::MODE_PIXELS
		);

		const auto &backgroundBrush = side ? tNameBackgroundBrush : ctNameBackgroundBrush;
		
		auto child = std::make_unique<ChildComponent>(
			commonResources, backgroundBrush, textBrush, *nameTextRenderer,
			side ? L"Terrorists"sv : L"Counter-terrorists"sv
		);
		(side ? rightNameDisplay : leftNameDisplay) = child.get();
		innerStack->children.emplace_back(StackComponentChild{
			{1, 1}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_RATIO},
			0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
			std::move(child)
		});

		auto bag = std::make_unique<BagComponent>(commonResources);
		auto winLoseDisplay = std::make_unique<WinLoseComponent>(
			commonResources,
			backgroundBlackBrush,
			backgroundBrush, side ? tScoreBackgroundBrush : ctScoreBackgroundBrush,
			textBrush, moneyGainBrush,
			*winLoseTextRenderer, *nameTextRenderer, *moneyGainTextRenderer, winLoseTransition
		);
		(side ? rightWinLoseDisplay : leftWinLoseDisplay) = winLoseDisplay.get();
		bag->children.emplace_back(std::move(winLoseDisplay));
		auto matchPointDisplay = std::make_unique<MatchPointComponent>(
			commonResources, backgroundBrush, textBrush, *nameTextRenderer, matchPointTransition
		);
		(side ? rightMatchPointDisplay : leftMatchPointDisplay) = matchPointDisplay.get();
		bag->children.emplace_back(std::move(matchPointDisplay));
		auto timeoutDisplay = std::make_unique<TimeoutComponent>(
			commonResources, backgroundBrush, textBrush, *nameTextRenderer, timeoutTransition
		);
		(side ? rightTimeoutDisplay : leftTimeoutDisplay) = timeoutDisplay.get();
		bag->children.emplace_back(std::move(timeoutDisplay));
		innerStack->children.emplace_back(StackComponentChild{
			{1, 1}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_RATIO},
			0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
			std::move(bag)
		});
		
		stack->children.emplace_back(StackComponentChild{
			{100, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
			0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
			std::move(innerStack)
		});
	};
	
	std::unique_ptr<ChildComponent> currentChild;
	
	currentChild = std::make_unique<ChildComponent>(
		commonResources, ctScoreBackgroundBrush, textBrush, *scoreTextRenderer, L"0"sv
	);
	leftScoreDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{40, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});

	makeNameStack(false);
	
	stack->children.emplace_back(StackComponentChild{
		{80, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::make_unique<ClockComponent>(commonResources)
	});

	makeNameStack(true);
	
	currentChild = std::make_unique<ChildComponent>(
		commonResources, tScoreBackgroundBrush, textBrush, *scoreTextRenderer, L"0"sv
	);
	rightScoreDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{40, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});

	ctNameDisplay = leftNameDisplay;
	tNameDisplay = rightNameDisplay;
	ctScoreDisplay = leftScoreDisplay;
	tScoreDisplay = rightScoreDisplay;

	leftNameWidth = &stack->children[1].size.width;
	rightNameWidth = &stack->children[3].size.width;
	
	container = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_VERTICAL, StackComponent::AXIS_INCREASE, 0.5f, StackComponent::MODE_RATIO
	);
	container->children.emplace_back(StackComponentChild{
		{0.5f, 28}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
		0.5f, StackComponentChild::MODE_RATIO,
		0, StackComponentChild::MODE_PIXELS,
		std::move(stack)
	});
	container->children.emplace_back(StackComponentChild{
		{0, 0}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_PIXELS},
		0, StackComponentChild::MODE_PIXELS,
		0, StackComponentChild::MODE_PIXELS,
		nullptr
	});
	container->children.emplace_back(StackComponentChild{
		{1, 24}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
		0.5f, StackComponentChild::MODE_RATIO,
		0, StackComponentChild::MODE_PIXELS,
		std::make_unique<BombTimerComponent>(commonResources)
	});
	paddingHeight = &container->children[1].size.height;

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToDataEvent("map"s, [this](JSON::dom::object &json) { receiveMapData(json); });
}

void TopBarComponent::receiveMapData(JSON::dom::object &json) {
	JSON::dom::object
		ct = json["team_ct"sv].value().get_object(),
		t = json["team_t"sv].value().get_object();
	JSON::simdjson_result<JSON::dom::element> value;
	value = ct["name"sv];
	ctNameDisplay->text = value.error() ? L"Counter-terrorists"s : Utils::widenString(value.value().get_string());
	ctScore = static_cast<int>(ct["score"sv].value().get_int64());
	ctTimeouts = static_cast<int>(ct["timeouts_remaining"sv].value().get_int64());
	ctScoreDisplay->text = std::to_wstring(ctScore);
	value = t["name"sv];
	tNameDisplay->text = value.error() ? L"Terrorists"s : Utils::widenString(value.value().get_string());
	tScore = static_cast<int>(t["score"sv].value().get_int64());
	tTimeouts = static_cast<int>(t["timeouts_remaining"sv].value().get_int64());
	tScoreDisplay->text = std::to_wstring(tScore);
}

void TopBarComponent::updateCtSide(const bool toTheLeft) {
	if (ctToTheLeft == toTheLeft) return;
	ctToTheLeft = toTheLeft;
	std::swap(leftNameDisplay->text, rightNameDisplay->text);
	std::swap(leftNameDisplay->backgroundBrush, rightNameDisplay->backgroundBrush);
	// It's just unreliable to try to guess whether to swap the scores, to resync press a spectator slot key.
	//std::swap(leftScoreDisplay->text, rightScoreDisplay->text);
	std::swap(leftScoreDisplay->backgroundBrush, rightScoreDisplay->backgroundBrush);
	std::swap(leftWinLoseDisplay->backgroundTeamBrush, rightWinLoseDisplay->backgroundTeamBrush);
	std::swap(leftWinLoseDisplay->teamBrush, rightWinLoseDisplay->teamBrush);
	std::swap(leftMatchPointDisplay->backgroundBrush, rightMatchPointDisplay->backgroundBrush);
	std::swap(leftTimeoutDisplay->backgroundBrush, rightTimeoutDisplay->backgroundBrush);
	std::swap(ctNameDisplay, tNameDisplay);
	std::swap(ctScoreDisplay, tScoreDisplay);
}

void TopBarComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	int firstPlayerIndex = commonResources.players.getFirstPlayerIndex();
	if (firstPlayerIndex != -1) updateCtSide(commonResources.players[firstPlayerIndex]->team);
	// (parentWidth:2 – 40 – 80 – 40) : 2.
	*leftNameWidth = *rightNameWidth = parentSize.width/4 - 80;

	auto computeGain = [](
		const bool team, const bool win, const int winningCondition, const RoundsData &roundsData
	) -> std::pair<int, int> {
		int oldLevel = 1, level = 1;
		const auto &rounds = roundsData.getRounds();
		const int currentRound = roundsData.getCurrentRound();
		for (int round = currentRound > 15 ? 15 : 0; round != currentRound; ++round) {
			oldLevel = level;
			if (rounds[round].first == team) {
				if (level != 0) --level;
			} else {
				if (level != 4) ++level;
			}
		}
		static const std::array<int, 4> WIN_MONEY_MAP = {3250, 3250, 3500, 3500};
		return {
			oldLevel,
			win
				? WIN_MONEY_MAP[winningCondition] + (winningCondition == 2 ? 300 : 0)
				: 1400 + 500*oldLevel + (winningCondition == 3 ? 800 : 0)
		};
	};
	auto computeStreak = [](const bool team, const RoundsData &roundsData) {
		const auto &rounds = roundsData.getRounds();
		const int currentRound = roundsData.getCurrentRound();
		bool currentTeam = currentRound > 15 ? !team : team;
		const int firstCap = currentRound > 15 ? 15 : currentRound;
		int streak = 0;
		for (int round = 0; round != firstCap; ++round) {
			if (rounds[round].first == currentTeam) ++streak;
			else streak = 0;
		}
		if (currentRound > 15) for (int round = 15; round != currentRound; ++round) {
			if (rounds[round].first == team) ++streak;
			else streak = 0;
		}
		return streak;
	};

	const auto &rounds = commonResources.rounds;
	const bool currentWinLoseShown
		= rounds.getCurrentPhase() == RoundsData::Phase::OVER
		&& rounds.getCurrentRound() == rounds.getRounds().size();
	if (currentWinLoseShown != winLoseShown) {
		winLoseShown = currentWinLoseShown;
		if (winLoseShown) {
			const auto &result = rounds.getRounds().back();
			int winningCondition = static_cast<int>(result.second);
			bool win = result.first == ctToTheLeft;
			auto gain = computeGain(ctToTheLeft, win, winningCondition, rounds);
			int streak = computeStreak(ctToTheLeft, rounds);
			leftWinLoseDisplay->winIconIndex = win ? RoundsData::iconMap[winningCondition] : -1;
			leftWinLoseDisplay->lossBonusLevel = gain.first;
			leftWinLoseDisplay->moneyGain = L"+"s + std::to_wstring(gain.second) + L" $"s;
			if (win) leftWinLoseDisplay->streak = streak > 1 ? L"🔥 "s + std::to_wstring(streak) : L""s;
			win = !win;
			gain = computeGain(!ctToTheLeft, win, winningCondition, rounds);
			streak = computeStreak(!ctToTheLeft, rounds);
			rightWinLoseDisplay->winIconIndex = win ? RoundsData::iconMap[winningCondition] : -1;
			rightWinLoseDisplay->lossBonusLevel = gain.first;
			rightWinLoseDisplay->moneyGain = L"+" + std::to_wstring(gain.second) + L" $"s;
			if (win) rightWinLoseDisplay->streak = streak > 1 ? L"🔥 "s + std::to_wstring(streak) : L""s;
			winLoseTransition.transition(1);
		} else {
			winLoseTransition.transition(0);
		}
	}
	const RoundsData::Phase currentPhase = rounds.getCurrentPhase();
	const bool currentTimeoutShown
		= currentPhase == RoundsData::Phase::TIMEOUT_CT || currentPhase == RoundsData::Phase::TIMEOUT_T;
	if (currentTimeoutShown != timeoutShown) {
		timeoutShown = currentTimeoutShown;
		if (timeoutShown) {
			TimeoutComponent
				*ctDisplay = ctToTheLeft ? leftTimeoutDisplay : rightTimeoutDisplay,
				*tDisplay = ctToTheLeft ? rightTimeoutDisplay : leftTimeoutDisplay;
			if (currentPhase == RoundsData::Phase::TIMEOUT_CT) {
				ctDisplay->timeoutsRemaining = ctTimeouts;
				tDisplay->timeoutsRemaining = -1;
			} else {
				ctDisplay->timeoutsRemaining = -1;
				tDisplay->timeoutsRemaining = tTimeouts;
			}
			timeoutTransition.transition(1);
		} else {
			timeoutTransition.transition(0);
		}
	}
	const bool currentMatchPointShown = rounds.isBeginningOfRound() && !timeoutShown;
	if (currentMatchPointShown != matchPointShown) {
		matchPointShown = currentMatchPointShown;
		if (matchPointShown) {
			const int
				round = rounds.getCurrentRound(),
				threshold = round > 30 ? 15 + (round - 25) / 6 * 3 : 15,
				difference = std::abs(ctScore - tScore);
			if (ctScore == threshold || tScore == threshold) {
				if (ctToTheLeft == (ctScore == threshold)) {
					leftMatchPointDisplay->matchPoints = difference;
					rightMatchPointDisplay->matchPoints = 0;
				} else {
					leftMatchPointDisplay->matchPoints = 0;
					rightMatchPointDisplay->matchPoints = difference;
				}
				matchPointTransition.transition(1);
			}
		} else {
			matchPointTransition.transition(0);
		}
	}
	*paddingHeight = winLoseTransition.getValue() * 28;
	
	container->paint(transform, parentSize);
}

} // namespace CsgoHud