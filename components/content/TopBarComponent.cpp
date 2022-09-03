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
#include "resources/CommonResources.h"

#include "components/content/TopBarComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

static const int FONT_SIZE = 24;
static const float TEXT_OFFSET = -1;

// == TopBarComponent::ChildComponent ==

TopBarComponent::ChildComponent::ChildComponent(
	CommonResources &commonResources,
	const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
	const winrt::com_ptr<IDWriteTextFormat> &textFormat,
	std::wstring_view text
):
	Component(commonResources),
	backgroundBrush(backgroundBrush), textBrush(textBrush), textFormat(textFormat),
	text(text)
{}

void TopBarComponent::ChildComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());
	renderTarget.DrawText(
		text.c_str(), static_cast<UINT32>(text.size()),
		textFormat.get(),
		{0, (parentSize.height-FONT_SIZE)/2 + TEXT_OFFSET, parentSize.width, parentSize.height},
		textBrush.get(),
		D2D1_DRAW_TEXT_OPTIONS_NO_SNAP, DWRITE_MEASURING_MODE_NATURAL
	);

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

// == TopBarComponent ==

TopBarComponent::TopBarComponent(CommonResources &commonResources): Component(commonResources) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 0.5f}, ctNameBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctScoreBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.5f}, tNameBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tScoreBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		FONT_SIZE, L"", nameTextFormat.put()
	);
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		FONT_SIZE, L"", scoreTextFormat.put()
	);
	nameTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	scoreTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	auto stack = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_HORIZONTAL, StackComponent::AXIS_INCREASE, 0.f, StackComponent::MODE_PIXELS
	);

	std::unique_ptr<ChildComponent> currentChild;
	currentChild = std::make_unique<ChildComponent>(
		commonResources, ctScoreBackgroundBrush, textBrush, scoreTextFormat, L"0"sv
	);
	leftScoreDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{40, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});
	currentChild = std::make_unique<ChildComponent>(
		commonResources, ctNameBackgroundBrush, textBrush, nameTextFormat, L"Counter-terrorists"sv
	);
	leftNameDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{100, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});
	
	stack->children.emplace_back(StackComponentChild{
		{80, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::make_unique<ClockComponent>(commonResources)
	});
	
	currentChild = std::make_unique<ChildComponent>(
		commonResources, tNameBackgroundBrush, textBrush, nameTextFormat, L"Terrorists"sv
	);
	rightNameDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{1000, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
	});
	currentChild = std::make_unique<ChildComponent>(
		commonResources, tScoreBackgroundBrush, textBrush, scoreTextFormat, L"0"sv
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
		{1, 24}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
		0.5f, StackComponentChild::MODE_RATIO,
		0, StackComponentChild::MODE_PIXELS,
		std::make_unique<BombTimerComponent>(commonResources)
	});

	auto &eventBus = commonResources.eventBus;
	eventBus.listenToDataEvent("player"s, [this](const JSON &json) { receivePlayerData(json); });
	eventBus.listenToDataEvent("map"s, [this](const JSON &json) { receiveMapData(json); });
}

void TopBarComponent::receivePlayerData(const JSON &json) {
	if (!json.contains("spectarget"s) || json["spectarget"s].get<std::string>()[0] == 'f'/*ree*/) return;
	const char slot = json["observer_slot"s].get<int>();
	updateCtSide((slot >= 1 && slot <= 5) == (json["team"s].get<std::string>()[0] == 'C'/*T*/));
}

void TopBarComponent::receiveMapData(const JSON &json) {
	ctScoreDisplay->text = std::to_wstring(json["team_ct"s]["score"s].get<int>());
	tScoreDisplay->text = std::to_wstring(json["team_t"s]["score"s].get<int>());
}

void TopBarComponent::updateCtSide(const bool toTheLeft) {
	if (ctToTheLeft == toTheLeft) return;
	ctToTheLeft = toTheLeft;
	std::swap(leftNameDisplay->text, rightNameDisplay->text);
	std::swap(leftNameDisplay->backgroundBrush, rightNameDisplay->backgroundBrush);
	std::swap(leftScoreDisplay->text, rightScoreDisplay->text);
	std::swap(leftScoreDisplay->backgroundBrush, rightScoreDisplay->backgroundBrush);
	std::swap(ctNameDisplay, tNameDisplay);
	std::swap(ctScoreDisplay, tScoreDisplay);
}

void TopBarComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	// (parentWidth:2 – 40 – 80 – 40) : 2.
	*leftNameWidth = *rightNameWidth = parentSize.width/4 - 80;
	container->paint(transform, parentSize);
}

} // namespace CsgoHud