#include <string>
#include <string_view>

#include "pch.h"

#include "components/base/Component.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "text/TextRenderer.h"
#include "utils/CommonConstants.h"

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
	const TextRenderer &winTextRenderer,
	const TextRenderer &streakTextRenderer,
	const TextRenderer &moneyGainTextRenderer,
	const TransitionedValue &transition
):
	Component(commonResources),
	backgroundBlackBrush(backgroundBlackBrush), backgroundTeamBrush(backgroundTeamBrush),
	teamBrush(teamBrush), textBrush(textBrush), moneyGainBrush(moneyGainBrush),
	winTextRenderer(winTextRenderer), streakTextRenderer(streakTextRenderer),
	moneyGainTextRenderer(moneyGainTextRenderer), transition(transition)
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
		winTextRenderer.draw(	
			L"WIN"sv, {12 + parentSize.height * 3 / 4, 0, parentSize.width, parentSize.height}, textBrush
		);
		if (!streak.empty()) {
			const D2D1_RECT_F bounds = {0, 0, parentSize.width, parentSize.height};
			auto textLayout = streakTextRenderer.prepareLayout(streak, bounds);
			textLayout->SetFontSize(12, {0, 1});
			streakTextRenderer.drawPreparedLayout(textLayout, bounds, textBrush);
		}
	}
	moneyGainTextRenderer.draw(moneyGain, {0, 0, parentSize.width - 8, parentSize.height}, moneyGainBrush);
	if (transiting) renderTarget.PopLayer();
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

// == TopBarComponent::MatchPointComponent ==

TopBarComponent::MatchPointComponent::MatchPointComponent(
	CommonResources &commonResources,
	const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
	const TextRenderer &textRenderer,
	const TransitionedValue &transition
):
	Component(commonResources),
	backgroundBrush(backgroundBrush), textBrush(textBrush), textRenderer(textRenderer),
	transition(transition)
{
	commonResources.renderTarget->CreateLayer(layer.put());
}

void TopBarComponent::MatchPointComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = transition.getValue();
	if (transitionValue == 0 || matchPoints == 0) return;
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
	const D2D1_RECT_F bounds = {0, 0, parentSize.width, parentSize.height};
	renderTarget.FillRectangle(bounds, backgroundBrush.get());
	if (matchPoints == 1) {
		textRenderer.draw(L"MATCH POINT"sv, bounds, textBrush);
	} else {
		const std::wstring numberString = std::to_wstring(matchPoints);
		auto textLayout = textRenderer.prepareLayout(numberString + L" MATCH POINTS"s, bounds);
		textLayout->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, {0, static_cast<UINT32>(numberString.size())});
		textRenderer.drawPreparedLayout(textLayout, bounds, textBrush);
	}
	if (transiting) renderTarget.PopLayer();
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

// == TopBarComponent::TimeoutComponent ==

TopBarComponent::TimeoutComponent::TimeoutComponent(
	CommonResources &commonResources,
	const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
	const TextRenderer &textRenderer,
	const TransitionedValue &transition
):
	Component(commonResources),
	backgroundBrush(backgroundBrush), textBrush(textBrush), textRenderer(textRenderer),
	transition(transition)
{
	commonResources.renderTarget->CreateLayer(layer.put());
}

void TopBarComponent::TimeoutComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = transition.getValue();
	if (transitionValue == 0 || timeoutsRemaining == -1) return;
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
	const D2D1_RECT_F bounds = {0, 0, parentSize.width, parentSize.height};
	renderTarget.FillRectangle(bounds, backgroundBrush.get());
	const std::wstring text = L"Timeout "s + std::to_wstring(timeoutsRemaining) + L" remaining"s;
	auto textLayout = textRenderer.prepareLayout(text, bounds);
	textLayout->SetFontSize(14, {8, static_cast<UINT32>(text.size() - 8)});
	textRenderer.drawPreparedLayout(textLayout, bounds, textBrush);
	if (transiting) renderTarget.PopLayer();
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud