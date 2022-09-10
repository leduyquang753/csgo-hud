#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pch.h"

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

// == TopBarComponent::ChildComponent ==

TopBarComponent::ChildComponent::ChildComponent(
	CommonResources &commonResources,
	const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
	const TextRenderer &textRenderer,
	std::wstring_view text
):
	Component(commonResources),
	backgroundBrush(backgroundBrush), textBrush(textBrush), textRenderer(textRenderer),
	text(text)
{}

void TopBarComponent::ChildComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());
	textRenderer.draw(text, {0, 0, parentSize.width, parentSize.height}, textBrush);

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

// == TopBarComponent::WinLoseComponent ==

TopBarComponent::WinLoseComponent::WinLoseComponent(
	CommonResources &commonResources,
	const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBlackBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundTeamBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &teamBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &moneyGainBrush,
	const TextRenderer &textRenderer,
	const TextRenderer &moneyGainTextRenderer,
	const TransitionedValue &transition
):
	Component(commonResources),
	backgroundBlackBrush(backgroundBlackBrush), backgroundTeamBrush(backgroundTeamBrush),
	teamBrush(teamBrush), textBrush(textBrush), moneyGainBrush(moneyGainBrush),
	textRenderer(textRenderer), moneyGainTextRenderer(moneyGainTextRenderer), transition(transition)
{
	commonResources.renderTarget->CreateLayer(layer.put());
}

void TopBarComponent::WinLoseComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = transition.getValue();
	if (transitionValue == 0) return;
	const bool transiting = transition.transiting();
	auto &renderTarget = *commonResources.renderTarget;
	D2D1::Matrix3x2F currentTransform = transform;
	if (transiting) {
		renderTarget.SetTransform(currentTransform);
		renderTarget.PushLayer(
			{
				{0, 0, parentSize.width, parentSize.height},
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				transitionValue, nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			layer.get()
		);
		currentTransform
			 = D2D1::Matrix3x2F::Translation(0, parentSize.height * (transitionValue-1)) * currentTransform;
	}
	renderTarget.SetTransform(currentTransform);
	renderTarget.FillRectangle(
		{0, 0, parentSize.width, parentSize.height},
		winIconIndex == -1 ? backgroundBlackBrush.get() : backgroundTeamBrush.get()
	);
	if (winIconIndex == -1) {
		const float
			length = parentSize.height * 3 / 4,
			top = parentSize.height * 3 / 8, bottom = parentSize.height * 5 / 8;
		for (int i = 0; i != 4; ++i) {
			const float x = 8 + (length + 4) * i;
			renderTarget.FillRectangle(
				{x, top, x + length, bottom},
				i < lossBonusLevel ? teamBrush.get() : backgroundTeamBrush.get()
			);
		}
	} else {
		const float scale = parentSize.height / CommonConstants::ICON_HEIGHT * 3 / 4;
		renderTarget.SetTransform(
			D2D1::Matrix3x2F::Scale(scale, scale, {0, 0})
			* D2D1::Matrix3x2F::Translation(8, parentSize.height/8)
			* currentTransform
		);
		renderTarget.DrawImage(
			commonResources.icons[winIconIndex].source.get(), nullptr, nullptr,
			D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, D2D1_COMPOSITE_MODE_SOURCE_OVER
		);
		renderTarget.SetTransform(currentTransform);
		textRenderer.draw(	
			L"WIN"sv, {12 + parentSize.height * 3 / 4, 0, parentSize.width, parentSize.height}, textBrush
		);
	}
	moneyGainTextRenderer.draw(moneyGain, {0, 0, parentSize.width - 8, parentSize.height}, moneyGainBrush);
	if (transiting) renderTarget.PopLayer();
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

// == TopBarComponent ==

TopBarComponent::TopBarComponent(CommonResources &commonResources):
	Component(commonResources),
	winLoseTransition(
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

	std::unique_ptr<StackComponent> currentStack;
	std::unique_ptr<ChildComponent> currentChild;
	std::unique_ptr<WinLoseComponent> currentWinLoseDisplay;
	
	currentChild = std::make_unique<ChildComponent>(
		commonResources, ctScoreBackgroundBrush, textBrush, *scoreTextRenderer, L"0"sv
	);
	leftScoreDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{40, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});

	currentStack = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_VERTICAL, StackComponent::AXIS_INCREASE, 0.f, StackComponent::MODE_PIXELS
	);
	currentChild = std::make_unique<ChildComponent>(
		commonResources, ctNameBackgroundBrush, textBrush, *nameTextRenderer, L"Counter-terrorists"sv
	);
	leftNameDisplay = currentChild.get();
	currentStack->children.emplace_back(StackComponentChild{
		{1, 1}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});
	currentWinLoseDisplay = std::make_unique<WinLoseComponent>(
		commonResources,
		backgroundBlackBrush, ctNameBackgroundBrush, ctScoreBackgroundBrush, textBrush, moneyGainBrush,
		*winLoseTextRenderer, *moneyGainTextRenderer, winLoseTransition
	);
	leftWinLoseDisplay = currentWinLoseDisplay.get();
	currentStack->children.emplace_back(StackComponentChild{
		{1, 1}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentWinLoseDisplay)
	});
	stack->children.emplace_back(StackComponentChild{
		{100, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentStack)
	});
	
	stack->children.emplace_back(StackComponentChild{
		{80, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::make_unique<ClockComponent>(commonResources)
	});
	
	currentStack = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_VERTICAL, StackComponent::AXIS_INCREASE, 0.f, StackComponent::MODE_PIXELS
	);
	currentChild = std::make_unique<ChildComponent>(
		commonResources, tNameBackgroundBrush, textBrush, *nameTextRenderer, L"Terrorists"sv
	);
	rightNameDisplay = currentChild.get();
	currentStack->children.emplace_back(StackComponentChild{
		{1, 1}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});
	currentWinLoseDisplay = std::make_unique<WinLoseComponent>(
		commonResources,
		backgroundBlackBrush, tNameBackgroundBrush, tScoreBackgroundBrush, textBrush, moneyGainBrush,
		*winLoseTextRenderer, *moneyGainTextRenderer, winLoseTransition
	);
	rightWinLoseDisplay = currentWinLoseDisplay.get();
	currentStack->children.emplace_back(StackComponentChild{
		{1, 1}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentWinLoseDisplay)
	});
	stack->children.emplace_back(StackComponentChild{
		{100, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentStack)
	});
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
	eventBus.listenToDataEvent(
		"phase_countdowns"s, [this](JSON::dom::object &json) { receivePhaseData(json); }
	);
}

void TopBarComponent::receiveMapData(JSON::dom::object &json) {
	JSON::dom::object
		ct = json["team_ct"sv].value().get_object(),
		t = json["team_t"sv].value().get_object();
	JSON::simdjson_result<JSON::dom::element> value;
	value = ct["name"sv];
	ctNameDisplay->text = value.error() ? L"Counter-terrorists"s : Utils::widenString(value.value().get_string());
	ctScoreDisplay->text = std::to_wstring(ct["score"sv].value().get_int64());
	value = t["name"sv];
	tNameDisplay->text = value.error() ? L"Terrorists"s : Utils::widenString(value.value().get_string());
	tScoreDisplay->text = std::to_wstring(t["score"sv].value().get_int64());
}

void TopBarComponent::receivePhaseData(JSON::dom::object &json) {
	isOverPhase = json["phase"sv].value().get_string().value()[0] == 'o'/*ver*/;
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

	const auto &rounds = commonResources.rounds;
	const bool currentWinLoseShown = isOverPhase && rounds.getCurrentRound() == rounds.getRounds().size();
	if (currentWinLoseShown != winLoseShown) {
		winLoseShown = currentWinLoseShown;
		if (winLoseShown) {
			const auto &result = rounds.getRounds().back();
			int winningCondition = static_cast<int>(result.second);
			bool win = result.first == ctToTheLeft;
			auto gain = computeGain(ctToTheLeft, win, winningCondition, rounds);
			leftWinLoseDisplay->winIconIndex = win ? RoundsData::iconMap[winningCondition] : -1;
			leftWinLoseDisplay->lossBonusLevel = gain.first;
			leftWinLoseDisplay->moneyGain = L"+"s + std::to_wstring(gain.second) + L" $"s;
			win = !win;
			gain = computeGain(!ctToTheLeft, win, winningCondition, rounds);
			rightWinLoseDisplay->winIconIndex = win ? RoundsData::iconMap[winningCondition] : -1;
			rightWinLoseDisplay->lossBonusLevel = gain.first;
			rightWinLoseDisplay->moneyGain = L"+" + std::to_wstring(gain.second) + L" $"s;
			winLoseTransition.transition(1);
		} else {
			winLoseTransition.transition(0);
		}
	}
	*paddingHeight = winLoseTransition.getValue() * 28;
	
	container->paint(transform, parentSize);
}

} // namespace CsgoHud