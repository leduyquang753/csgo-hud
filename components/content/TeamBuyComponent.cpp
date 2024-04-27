#include "pch.h"

#include <algorithm>
#include <string>
#include <string_view>

#include "components/base/Component.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "text/NormalTextRenderer.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/Utils.h"

#include "components/content/TeamBuyComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == TeamBuyComponent ==

TeamBuyComponent::TeamBuyComponent(
	CommonResources &commonResources,
	const TransitionedValue &fadingTransition, const TransitionedValue &slidingTransition
):
	Component(commonResources), fadingTransition(fadingTransition), slidingTransition(slidingTransition)
{
	auto &renderTarget = *commonResources.renderTarget;
	const auto &colors = commonResources.configuration.colors;
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, backgroundBrush.put());
	renderTarget.CreateSolidColorBrush(colors.ctSecondary, ctBrush.put());
	renderTarget.CreateSolidColorBrush(colors.tSecondary, tBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());

	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	const auto fontFamily = commonResources.configuration.fontFamily.c_str();
	const float
		fontOffsetRatio = commonResources.configuration.fontOffsetRatio,
		fontLineHeightRatio = commonResources.configuration.fontLineHeightRatio;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	titleTextRenderer.emplace(commonResources, textFormat, fontOffsetRatio, fontLineHeightRatio);

	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		20, L"", textFormat.put()
	);
	amountTextRenderer.emplace(commonResources, textFormat, fontOffsetRatio, fontLineHeightRatio);
	amountTextFormat = textFormat;

	renderTarget.CreateLayer(layer.put());
}

void TeamBuyComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = 1 - fadingTransition.getValue();
	const float slidingTransitionValue = slidingTransition.getValue();
	if (transitionValue == 0 || slidingTransitionValue == 1) return;
	const bool
		alphaTransiting = fadingTransition.transiting(),
		slideTransiting = slidingTransition.transiting();

	const auto &players = commonResources.players;
	const int firstPlayerIndex = players.getFirstPlayerIndex();
	if (firstPlayerIndex == -1) return;
	const bool leftTeam = players[firstPlayerIndex]->team;

	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	if (alphaTransiting || slideTransiting) renderTarget.PushLayer(
		{
			{0, 0, parentSize.width, parentSize.height},
			nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
			transitionValue, nullptr,
			D2D1_LAYER_OPTIONS_NONE
		},
		layer.get()
	);
	if (slideTransiting) renderTarget.SetTransform(
		D2D1::Matrix3x2F::Translation(0, parentSize.height * slidingTransitionValue) * transform
	);

	const float
		innerLeft = 8,
		innerRight = parentSize.width - 8,
		innerWidth = parentSize.width - 16,
		amountLeft = 16,
		amountRight = parentSize.width - 16,
		buyAmountTop = 8,
		buyAmountBottom = 36,
		equipmentValueTop = parentSize.height - 36,
		equipmentValueBottom = parentSize.height - 8;

	renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());

	auto computeBuyAmount = [this, &players](const bool team) {
		int amount = 0;
		for (int i = 0; i != 10; ++i) {
			const auto &player = players[i];
			if (player && player->team == team) amount += std::max(0, player->startingMoney - player->money);
		}
		return amount;
	};
	auto computeEquipmentValue = [this, &players](const bool team) {
		int value = 0;
		for (int i = 0; i != 10; ++i) {
			const auto &player = players[i];
			if (player && player->team == team) value += player->equipmentValue;
		}
		return value;
	};

	const int
		leftBuyAmount = computeBuyAmount(leftTeam),
		rightBuyAmount = computeBuyAmount(!leftTeam),
		leftEquipmentValue = computeEquipmentValue(leftTeam),
		rightEquipmentValue = computeEquipmentValue(!leftTeam);

	auto drawGraph = [this, &renderTarget, leftTeam, innerLeft, innerRight, innerWidth](
		const int leftAmount, const int rightAmount, const float top, const float bottom
	) {
		const int total = leftAmount + rightAmount;
		if (total == 0) return;
		const float middle = innerLeft + innerWidth * static_cast<float>(leftAmount) / total;
		renderTarget.FillRectangle({innerLeft, top, middle, bottom}, leftTeam ? ctBrush.get() : tBrush.get());
		renderTarget.FillRectangle({middle, top, innerRight, bottom}, leftTeam ? tBrush.get() : ctBrush.get());
	};
	drawGraph(leftBuyAmount, rightBuyAmount, buyAmountTop, buyAmountBottom);
	drawGraph(leftEquipmentValue, rightEquipmentValue, equipmentValueTop, equipmentValueBottom);

	titleTextRenderer->draw(
		L"BUY AMOUNT"sv, {innerLeft, buyAmountTop, innerRight, buyAmountBottom}, textBrush
	);
	titleTextRenderer->draw(
		L"EQUIPMENT VALUE"sv, {innerLeft, equipmentValueTop, innerRight, equipmentValueBottom}, textBrush
	);
	amountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	amountTextRenderer->draw(
		Utils::formatMoneyAmount(leftBuyAmount, commonResources.configuration),
		{amountLeft, buyAmountTop, amountRight, buyAmountBottom},
		textBrush
	);
	amountTextRenderer->draw(
		Utils::formatMoneyAmount(leftEquipmentValue, commonResources.configuration),
		{amountLeft, equipmentValueTop, amountRight, equipmentValueBottom},
		textBrush
	);
	amountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	amountTextRenderer->draw(
		Utils::formatMoneyAmount(rightBuyAmount, commonResources.configuration),
		{amountLeft, buyAmountTop, amountRight, buyAmountBottom},
		textBrush
	);
	amountTextRenderer->draw(
		Utils::formatMoneyAmount(rightEquipmentValue, commonResources.configuration),
		{amountLeft, equipmentValueTop, amountRight, equipmentValueBottom},
		textBrush
	);

	if (alphaTransiting || slideTransiting) renderTarget.PopLayer();

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud