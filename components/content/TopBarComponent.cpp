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

// == TopBarComponent ==

TopBarComponent::TopBarComponent(CommonResources &commonResources): Component(commonResources) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 0.5f}, ctNameBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctScoreBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.5f}, tNameBackgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tScoreBackgroundBrush.put());
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

	auto stack = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_HORIZONTAL, StackComponent::AXIS_INCREASE, 0.f, StackComponent::MODE_PIXELS
	);

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
	currentChild = std::make_unique<ChildComponent>(
		commonResources, ctNameBackgroundBrush, textBrush, *nameTextRenderer, L"Counter-terrorists"sv
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
		commonResources, tNameBackgroundBrush, textBrush, *nameTextRenderer, L"Terrorists"sv
	);
	rightNameDisplay = currentChild.get();
	stack->children.emplace_back(StackComponentChild{
		{1000, 1}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_RATIO},
		0, StackComponentChild::MODE_PIXELS, 0, StackComponentChild::MODE_PIXELS,
		std::move(currentChild)
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
		{1, 24}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
		0.5f, StackComponentChild::MODE_RATIO,
		0, StackComponentChild::MODE_PIXELS,
		std::make_unique<BombTimerComponent>(commonResources)
	});

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
	ctScoreDisplay->text = std::to_wstring(ct["score"sv].value().get_int64());
	value = t["name"sv];
	tNameDisplay->text = value.error() ? L"Terrorists"s : Utils::widenString(value.value().get_string());
	tScoreDisplay->text = std::to_wstring(t["score"sv].value().get_int64());
}

void TopBarComponent::updateCtSide(const bool toTheLeft) {
	if (ctToTheLeft == toTheLeft) return;
	ctToTheLeft = toTheLeft;
	std::swap(leftNameDisplay->text, rightNameDisplay->text);
	std::swap(leftNameDisplay->backgroundBrush, rightNameDisplay->backgroundBrush);
	// It's just unreliable to try to guess whether to swap the scores, to resync press a spectator slot key.
	//std::swap(leftScoreDisplay->text, rightScoreDisplay->text);
	std::swap(leftScoreDisplay->backgroundBrush, rightScoreDisplay->backgroundBrush);
	std::swap(ctNameDisplay, tNameDisplay);
	std::swap(ctScoreDisplay, tScoreDisplay);
}

void TopBarComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	int firstPlayerIndex = commonResources.players.getFirstPlayerIndex();
	if (firstPlayerIndex != -1) updateCtSide(commonResources.players[firstPlayerIndex]->team);
	// (parentWidth:2 – 40 – 80 – 40) : 2.
	*leftNameWidth = *rightNameWidth = parentSize.width/4 - 80;
	container->paint(transform, parentSize);
}

} // namespace CsgoHud