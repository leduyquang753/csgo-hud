#include "pch.h"

#include <string>
#include <string_view>

#include "components/base/Component.h"
#include "data/IconStorage.h"
#include "data/PlayerData.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "text/NormalTextRenderer.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/CommonConstants.h"

#include "components/content/ActivePlayerComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == ActivePlayerComponent ==

ActivePlayerComponent::ActivePlayerComponent(
	CommonResources &commonResources,
	const TransitionedValue &fadingTransition, const TransitionedValue &slidingTransition
):
	Component(commonResources), fadingTransition(fadingTransition), slidingTransition(slidingTransition),
	selfTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	)
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, backgroundBlackBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 0.5f}, backgroundCtBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.5f}, backgroundTBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, whiteBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	const auto fontFamily = commonResources.configuration.fontFamily.c_str();
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		20, L"", textFormat.put()
	);
	winrt::com_ptr<IDWriteInlineObject> trimmingSign;
	static const DWRITE_TRIMMING TRIMMING_OPTIONS = {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
	textFormat->SetTrimming(&TRIMMING_OPTIONS, trimmingSign.get());
	textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	nameTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	statsTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	weaponTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		20, L"", textFormat.put()
	);
	bigNumberRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	bigNumberTextFormat = textFormat;

	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	smallNumberRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	renderTarget.CreateLayer(layer.put());
}

void ActivePlayerComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	static const float
		NAME_HEIGHT = 28,
		HEALTH_ARMOR_LENGTH = 144,
		BAR_WIDTH = 4,
		BAR_SPACING = 4,
		BAR_RESERVED_WIDTH = BAR_WIDTH + BAR_SPACING,
		AMMO_LENGTH = 74,
		AMMO_SEPARATION = 42,
		SPACING = 8;
	
	const float alpha = fadingTransition.getValue() * selfTransition.getValue();
	const int activeSlot = commonResources.players.getActivePlayerIndex();
	if (activeSlot == -1) {
		if (shown) {
			shown = false;
			selfTransition.transition(0);
		}
	} else {
		const auto &optionalPlayer = commonResources.players[activeSlot];
		if (optionalPlayer) {
			const bool currentShown = optionalPlayer->health != 0;
			if (currentShown != shown) {
				shown = currentShown;
				selfTransition.transition(shown ? 1.f : 0.f);
			}
			if (shown || alpha != 0) player = *optionalPlayer;
		} else {
			if (shown) {
				shown = false;
				selfTransition.transition(0);
			}
		}
	}
	const float slidingTransitionValue = slidingTransition.getValue();
	if (alpha == 0 || slidingTransitionValue == 1) return;
	const bool
		alphaTransiting = alpha != 1,
		slideTransiting = slidingTransition.transiting();
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	
	if (alphaTransiting || slideTransiting) renderTarget.PushLayer(
		{
			{0, 0, parentSize.width, parentSize.height},
			nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
			alpha, nullptr,
			D2D1_LAYER_OPTIONS_NONE
		},
		layer.get()
	);
	if (slideTransiting) renderTarget.SetTransform(
		D2D1::Matrix3x2F::Translation(0, parentSize.height * slidingTransitionValue) * transform
	);

	const float
		centerWidth = parentSize.width - (HEALTH_ARMOR_LENGTH + SPACING) * 2,
		centerLeft = (parentSize.width - centerWidth) / 2,
		centerRight = centerLeft + centerWidth,
		statWidth = centerWidth / 5,
		healthArmorRight = centerLeft - SPACING,
		healthArmorMiddle = healthArmorRight / 2,
		ammoLeft = centerRight + SPACING,
		ammoRight = ammoLeft + AMMO_LENGTH,
		ammoMiddle = ammoLeft + AMMO_SEPARATION,
		sideTop = parentSize.height - NAME_HEIGHT,
		sideMargin = NAME_HEIGHT / 8,
		sideInnerTop = parentSize.height - NAME_HEIGHT + sideMargin,
		sideInnerBottom = parentSize.height - sideMargin,
		sideInnerHeight = sideInnerBottom - sideInnerTop;

	renderTarget.FillRectangle(
		{centerLeft, 0, centerRight, NAME_HEIGHT},
		player.team ? backgroundCtBrush.get() : backgroundTBrush.get()
	);
	renderTarget.FillRectangle(	
		{centerLeft, NAME_HEIGHT, centerRight, parentSize.height}, backgroundBlackBrush.get()
	);

	nameTextRenderer->draw(player.name, {centerLeft + 8, 0, centerRight - 8, NAME_HEIGHT}, whiteBrush);

	const auto teamBrush = player.team ? ctBrush : tBrush;

	bigNumberTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	auto drawStat = [this, &parentSize, centerLeft, statWidth, &teamBrush](
		const int index, std::wstring_view title, const int amount
	) {
		const float
			left = centerLeft + index * statWidth,
			right = left + statWidth;
		statsTextRenderer->draw(title, {left, NAME_HEIGHT + 4, right, NAME_HEIGHT + 20}, teamBrush);
		bigNumberRenderer->draw(
			std::to_wstring(amount), {left, parentSize.height - 28, right, parentSize.height - 4}, whiteBrush
		);
	};
	drawStat(0, L"K"sv, player.totalKills);
	drawStat(1, L"A"sv, player.totalAssists);
	drawStat(2, L"D"sv, player.totalDeaths);
	drawStat(3, L"MVP"sv, player.totalMvps);
	drawStat(4, L"PTS"sv, player.totalScore);

	renderTarget.FillRectangle({0, sideTop, healthArmorRight, parentSize.height}, teamBrush.get());

	const auto bitmap = commonResources.icons.getBitmap();
	auto drawIcon = [this, &renderTarget, sideInnerHeight, bitmap](const int index, const float x, const float y) {
		const auto &icon = commonResources.icons[index];
		renderTarget.DrawBitmap(
			bitmap,
			{x, y, x + sideInnerHeight, y + sideInnerHeight},
			1, D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, icon.floatBounds, nullptr
		);
	};
	drawIcon(IconStorage::INDEX_HEALTH, sideMargin, sideInnerTop);
	drawIcon(
		player.armor != 0 && player.hasHelmet ? IconStorage::INDEX_FULL_ARMOR : IconStorage::INDEX_KEVLAR,
		healthArmorMiddle + sideMargin, sideInnerTop
	);
	
	bigNumberTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	bigNumberRenderer->draw(
		std::to_wstring(player.health),
		{0, sideTop, healthArmorMiddle - sideMargin - BAR_RESERVED_WIDTH, parentSize.height},
		whiteBrush
	);
	bigNumberRenderer->draw(
		std::to_wstring(player.armor),
		{healthArmorMiddle, sideTop, healthArmorRight - sideMargin - BAR_RESERVED_WIDTH, parentSize.height},
		whiteBrush
	);

	if (player.health != 0) renderTarget.FillRectangle(
		{
			healthArmorMiddle - sideMargin - BAR_WIDTH,
			sideInnerTop + sideInnerHeight * (1 - player.health / 100.f),
			healthArmorMiddle - sideMargin, sideInnerBottom
		},
		whiteBrush.get()
	);
	if (player.armor != 0) renderTarget.FillRectangle(
		{
			healthArmorRight - sideMargin - BAR_WIDTH,
			sideInnerTop + sideInnerHeight * (1 - player.armor / 100.f),
			healthArmorRight - sideMargin, sideInnerBottom
		},
		whiteBrush.get()
	);

	if (player.health != 0 && player.activeSlot != PlayerData::SLOT_NONE) {
		const bool gunActive = player.activeSlot < 3;
		std::wstring activeWeaponName;
		switch (player.activeSlot) {
			case PlayerData::SLOT_PRIMARY_GUN:
				activeWeaponName = commonResources.weaponTypes.displayNameMap[player.primaryGun->type];
				break;
			case PlayerData::SLOT_SECONDARY_GUN:
				activeWeaponName = commonResources.weaponTypes.displayNameMap[player.secondaryGun->type];
				break;
			case PlayerData::SLOT_KNIFE:
				activeWeaponName = L"Knife"s;
				break;
			case PlayerData::SLOT_ZEUS:
				activeWeaponName = L"Zeus x27"s;
				break;
			case PlayerData::SLOT_GRENADE_0:
			case PlayerData::SLOT_GRENADE_1:
			case PlayerData::SLOT_GRENADE_2:
			case PlayerData::SLOT_GRENADE_3: {
				auto optionalGrenade = player.grenades[player.activeSlot - PlayerData::SLOT_GRENADE_0];
				if (optionalGrenade)
					activeWeaponName = commonResources.weaponTypes.displayNameMap[*optionalGrenade];
				break;
			}
			case PlayerData::SLOT_C4:
				activeWeaponName = L"C4"s;
				break;
		}
		const float weaponNameBottom = gunActive ? parentSize.height - NAME_HEIGHT - 4 : parentSize.height - 2;
		weaponTextRenderer->draw(
			activeWeaponName, {ammoLeft, weaponNameBottom - 14, parentSize.width, weaponNameBottom}, whiteBrush
		);
			
		if (gunActive) {
			const auto &gun
				= player.activeSlot == PlayerData::SLOT_PRIMARY_GUN ? *player.primaryGun : *player.secondaryGun;
			renderTarget.FillRectangle({ammoLeft, sideTop, ammoRight, parentSize.height}, teamBrush.get());
			if (gun.roundsInClip != 0) renderTarget.FillRectangle(
				{
					ammoMiddle,
					sideInnerTop + sideInnerHeight * (1 - static_cast<float>(gun.roundsInClip) / gun.clipSize),
					ammoMiddle + BAR_WIDTH, sideInnerBottom
				},
				whiteBrush.get()
			);
			bigNumberRenderer->draw(
				std::to_wstring(gun.roundsInClip),
				{ammoLeft, sideTop, ammoMiddle - BAR_SPACING, parentSize.height},
				whiteBrush
			);
			if (gun.spareRounds != 0) smallNumberRenderer->draw(
				std::to_wstring(gun.roundsInClip + gun.spareRounds),
				{ammoMiddle + BAR_RESERVED_WIDTH, sideInnerBottom - 15, ammoRight, sideInnerBottom},
				whiteBrush
			);
		}
	}

	if (alphaTransiting || slideTransiting) renderTarget.PopLayer();

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud