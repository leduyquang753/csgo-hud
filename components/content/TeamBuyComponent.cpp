#include <string>
#include <string_view>

#include "pch.h"

#include "components/base/Component.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "text/NormalTextRenderer.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/CommonConstants.h"

#include "components/content/TeamBuyComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == TeamBuyComponent ==

TeamBuyComponent::TeamBuyComponent(CommonResources &commonResources, const TransitionedValue &masterTransition):
	Component(commonResources), masterTransition(masterTransition)
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, backgroundBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 0.5f}, ctBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.5f}, tBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	titleTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		20, L"", textFormat.put()
	);
	amountTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	amountTextFormat = textFormat;

	renderTarget.CreateLayer(layer.put());
}

void TeamBuyComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = 1 - masterTransition.getValue();
	if (transitionValue == 0) return;
	const bool transiting = masterTransition.transiting();

	const auto &players = commonResources.players;
	const int firstPlayerIndex = players.getFirstPlayerIndex();
	if (firstPlayerIndex == -1) return;
	const bool leftTeam = players[firstPlayerIndex]->team;
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	if (transiting) renderTarget.PushLayer(
		{
			{0, 0, parentSize.width, parentSize.height},
			nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
			transitionValue, nullptr,
			D2D1_LAYER_OPTIONS_NONE
		},
		layer.get()
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
			if (player && player->team == team) amount += player->startingMoney - player->money;
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
		const float middle = innerLeft + innerWidth * static_cast<float>(leftAmount) / rightAmount;
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
		std::to_wstring(leftBuyAmount) + L" $"s,
		{amountLeft, buyAmountTop, amountRight, buyAmountBottom},
		textBrush
	);
	amountTextRenderer->draw(
		std::to_wstring(leftEquipmentValue) + L" $"s,
		{amountLeft, equipmentValueTop, amountRight, equipmentValueBottom},
		textBrush
	);
	amountTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	amountTextRenderer->draw(
		std::to_wstring(rightBuyAmount) + L" $"s,
		{amountLeft, buyAmountTop, amountRight, buyAmountBottom},
		textBrush
	);
	amountTextRenderer->draw(
		std::to_wstring(rightEquipmentValue) + L" $"s,
		{amountLeft, equipmentValueTop, amountRight, equipmentValueBottom},
		textBrush
	);

	if (transiting) renderTarget.PopLayer();

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud