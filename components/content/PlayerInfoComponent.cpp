#include "pch.h"

#include <algorithm>
#include <string>

#include "components/base/Component.h"
#include "data/IconStorage.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/LinearMovementFunction.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "components/content/PlayerInfoComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == PlayerInfoComponent ==

PlayerInfoComponent::PlayerInfoComponent(
	CommonResources &commonResources, const bool rightSide, const Resources &resources
):
	Component(commonResources), resources(resources), rightSide(rightSide),
	activeTransition(
		commonResources,
		std::make_unique<CubicBezierMovementFunction>(
			std::vector<D2D1_POINT_2F>{{{0, 0}, {0.25f, 0.1f}, {0.25f, 1}, {300, 1}}}
		),
		300, 0
	),
	healthTransition(
		commonResources,
		std::make_unique<LinearMovementFunction>(std::vector<D2D1_POINT_2F>{{{0, 0}, {300, 1}}}),
		300, 0
	)
{
	commonResources.renderTarget->CreateLayer(statsLayer.put());
	commonResources.eventBus.listenToTimeEvent([this](const int timePassed) { advanceTime(timePassed); });
}

void PlayerInfoComponent::advanceTime(const int timePassed) {
	if (healthDecayTime != 0) {
		if (healthDecayTime <= timePassed) {
			healthTransition.transition(static_cast<float>(currentHealth));
			healthDecayTime = 0;
		} else {
			healthDecayTime -= timePassed;
		}
	}
}

void PlayerInfoComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	static const float
		MAX_PRIMARY_GUN_LENGTH = 440,
		MAX_SECONDARY_GUN_LENGTH = 280,
		HEALTH_LENGTH = 27,
		SLOT_LENGTH = 30,
		SLOT_MARGIN = 4,
		SLOT_RENDER_LENGTH = SLOT_LENGTH - SLOT_MARGIN,
		UTILITY_MARGIN = 4,
		UTILITY_SPACING = 4,
		DEAD_ICON_POS = 10,
		PADDING = 4,
		KILL_COUNT_OFFSET = 8,
		KILL_COUNT_LENGTH = 27,
		KILL_COUNT_ICON_POS = 10,
		KD_CELL_LENGTH = 30,
		MONEY_CELL_LENGTH = 64,
		STAT_CELL_SPACING = 4,
		STATS_LENGTH = KD_CELL_LENGTH + MONEY_CELL_LENGTH + STAT_CELL_SPACING;

	if (active != wasActive) {
		wasActive = active;
		activeTransition.transition(active ? 1.f : 0.f);
	}
	
	if (index == -1) return;
	const auto &optionalPlayer = commonResources.players[index];
	if (!optionalPlayer) return;
	const auto &player = *optionalPlayer;

	if (lastIndex == -1 || player.steamId != lastPlayerId) {
		lastPlayerId = player.steamId;
		oldHealth = 0;
		healthDecayTime = 0;
		healthTransition.setValue(0);
		wasActive = false;
		activeTransition.setValue(0);
	}
	lastIndex = index;
		
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	
	const float
		verticalMiddle = parentSize.height / 2,
		scale = verticalMiddle / CommonConstants::ICON_HEIGHT,
		scaledMaxPrimaryGunLength = MAX_PRIMARY_GUN_LENGTH * scale;
	std::wstring textToRender;

	winrt::com_ptr<ID2D1SpriteBatch> spriteBatch;
	renderTarget.CreateSpriteBatch(spriteBatch.put());
	auto drawIcon = [this, &spriteBatch](
		const int index, const float x, const float y, const float scale,
		const bool active, const bool empty, const bool rightSide, const bool flipped
	) {
		const auto &icon = commonResources.icons[index];
		const float width = icon.width * scale, height = icon.height * scale;
		const D2D1_RECT_F destinationRect
			= rightSide
				? flipped
					? D2D1_RECT_F{x, y, x - width, y + height}
					: D2D1_RECT_F{x - width, y, x, y + height}
				: flipped
					? D2D1_RECT_F{x + width, y, x, y + height}
					: D2D1_RECT_F{x, y, x + width, y + height};
		spriteBatch->AddSprites(
			1, &destinationRect, &icon.bounds,
			active
				? empty ? &resources.weaponEmptyActiveColor : nullptr
				: empty ? &resources.weaponEmptyInactiveColor : &resources.weaponInactiveColor,
			nullptr, 0, 0, 0, 0
		);
	};
	
	winrt::com_ptr<ID2D1SolidColorBrush> backgroundBrush;
	const float in = activeTransition.getValue(), out = 1 - in;
	renderTarget.CreateSolidColorBrush(
		{
			resources.backgroundInactiveColor.r * out + resources.backgroundActiveColor.r * in,
			resources.backgroundInactiveColor.g * out + resources.backgroundActiveColor.g * in,
			resources.backgroundInactiveColor.b * out + resources.backgroundActiveColor.b * in,
			resources.backgroundInactiveColor.a * out + resources.backgroundActiveColor.a * in
		},
		backgroundBrush.put()
	);
	renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());

	if (player.health != 0) {
		auto drawEffect = [this, &renderTarget, &parentSize](const D2D1_COLOR_F &color, const int amount) {
			if (amount == 0) return;
			winrt::com_ptr<ID2D1SolidColorBrush> effectBrush;
			const float alpha = amount / 255.f;
			renderTarget.CreateSolidColorBrush({color.r, color.g, color.b, color.a * alpha}, effectBrush.put());
			renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, effectBrush.get());
		};
		drawEffect(resources.flashColor, player.flashAmount);
		drawEffect(resources.smokeColor, player.smokeAmount);
		drawEffect(resources.fireColor, player.fireAmount);
	}
	
	if (player.health > currentHealth) {
		currentHealth = oldHealth = player.health;
		healthTransition.setValue(static_cast<float>(oldHealth));
	} else if (player.health != currentHealth) {
		currentHealth = player.health;
		healthDecayTime = 2000;
	}
	
	{
		const float healthTransitionValue = healthTransition.getValue();
		if (healthTransitionValue != player.health) {
			renderTarget.FillRectangle(
				rightSide
					? D2D1_RECT_F{
						parentSize.width * (1 - healthTransitionValue / 100.f), 0,
						parentSize.width * (1 - player.health / 100.f), verticalMiddle
					}
					: D2D1_RECT_F{
						parentSize.width * player.health / 100, 0,
						parentSize.width * healthTransitionValue / 100, verticalMiddle
					},
				resources.healthBrush.get()
			);
		}
	}
	
	if (player.health == 0) {
		drawIcon(
			IconStorage::INDEX_DEAD,
			rightSide ? parentSize.width - DEAD_ICON_POS : DEAD_ICON_POS, parentSize.height / 4, scale,
			true, false, rightSide, false
		);
	} else {
		renderTarget.FillRectangle(
			rightSide
				? D2D1_RECT_F{parentSize.width * (1 - player.health / 100.f), 0, parentSize.width, verticalMiddle}
				: D2D1_RECT_F{0, 0, parentSize.width * player.health / 100, verticalMiddle},
			player.team ? resources.teamCtBrush.get() : resources.teamTBrush.get()
		);
		resources.boldTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		resources.boldTextRenderer.draw(
			std::to_wstring(player.health),
			rightSide
				? D2D1_RECT_F{
					parentSize.width - PADDING - HEALTH_LENGTH, 0, parentSize.width - PADDING, verticalMiddle
				}
				: D2D1_RECT_F{PADDING, 0, PADDING + HEALTH_LENGTH - 4, verticalMiddle},
			resources.textWhiteBrush
		);
		
		const float gunPosition = rightSide ? PADDING : parentSize.width - PADDING;
		if (player.primaryGun) drawIcon(
			player.primaryGun->type,
			gunPosition, 0, scale,
			player.activeSlot == PlayerData::SLOT_PRIMARY_GUN, player.primaryGun->roundsInClip == 0,
			!rightSide, rightSide
		);
		
		float secondRowPos = gunPosition;
		const float advanceDirection = rightSide ? 1.f : -1.f;
		if (player.secondaryGun) {
			drawIcon(
				player.secondaryGun->type,
				secondRowPos, verticalMiddle, scale,
				player.activeSlot == PlayerData::SLOT_SECONDARY_GUN, player.secondaryGun->roundsInClip == 0,
				!rightSide, rightSide
			);
			secondRowPos
				+= (commonResources.icons[player.secondaryGun->type].width * scale + UTILITY_MARGIN)
				* advanceDirection;
		}
		if (player.hasZeus) {
			drawIcon(
				IconStorage::INDEX_ZEUS, secondRowPos, verticalMiddle, scale,
				player.activeSlot == PlayerData::SLOT_ZEUS, false, !rightSide, rightSide
			);
			secondRowPos
				+= (commonResources.icons[IconStorage::INDEX_ZEUS].width * scale + UTILITY_MARGIN)
				* advanceDirection;
		}
		for (int grenadeSlot = 0; grenadeSlot != 4; ++grenadeSlot) {
			if (!player.grenades[grenadeSlot]) break;
			const int index = *player.grenades[grenadeSlot];
			drawIcon(
				index, secondRowPos, verticalMiddle, scale,
				player.activeSlot == PlayerData::SLOT_GRENADE_0 + grenadeSlot, false, !rightSide, false
			);
			secondRowPos += (commonResources.icons[index].width * scale + UTILITY_SPACING) * advanceDirection;
		}
		
		const float smallIconY = parentSize.height * 9 / 16;
		if (player.armor != 0) drawIcon(
			player.hasHelmet ? IconStorage::INDEX_FULL_ARMOR : IconStorage::INDEX_KEVLAR,
			rightSide ? parentSize.width - PADDING : PADDING, smallIconY, scale * 3 / 4,
			true, false, rightSide, false
		);
		if (player.hasC4OrDefuseKit) drawIcon(
			player.team ? IconStorage::INDEX_DEFUSE_KIT : IconStorage::INDEX_C4,
			rightSide ? parentSize.width - PADDING - 20 : PADDING + 20, smallIconY, scale * 3 / 4,
			player.activeSlot == PlayerData::SLOT_C4, false, rightSide, false
		);
	}
	renderTarget.SetTransform(transform);
	
	resources.normalTextFormat->SetTextAlignment(
		rightSide ? DWRITE_TEXT_ALIGNMENT_LEADING : DWRITE_TEXT_ALIGNMENT_TRAILING
	);
	const std::wstring playerNumber = std::to_wstring(
		commonResources.configuration.formatting.showTenthPlayerAsZero && index == 9 ? 0 : index + 1
	);
	resources.normalTextRenderer.draw(
		rightSide ? L"| "s + playerNumber : playerNumber + L" |"s,
		rightSide
			? D2D1_RECT_F{
				parentSize.width - PADDING - HEALTH_LENGTH - SLOT_RENDER_LENGTH, 0, parentSize.width, verticalMiddle
			}
			: D2D1_RECT_F{
				0, 0, PADDING + HEALTH_LENGTH + SLOT_RENDER_LENGTH, verticalMiddle
			},
		resources.textWhiteBrush
	);
	resources.normalTextFormat->SetTextAlignment(
		rightSide ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_LEADING
	);
	resources.normalTextRenderer.draw(
		player.name,
		player.health == 0 || !player.primaryGun
			? rightSide
				? D2D1_RECT_F{
					PADDING, 0, parentSize.width - PADDING - HEALTH_LENGTH - SLOT_LENGTH, verticalMiddle
				}
				: D2D1_RECT_F{
					PADDING + HEALTH_LENGTH + SLOT_LENGTH, 0, parentSize.width - PADDING, verticalMiddle
				}
			: rightSide
				? D2D1_RECT_F{
					PADDING + scaledMaxPrimaryGunLength, 0,
					parentSize.width - PADDING - HEALTH_LENGTH - SLOT_LENGTH, verticalMiddle
				}
				: D2D1_RECT_F{
					PADDING + HEALTH_LENGTH + SLOT_LENGTH, 0,
					parentSize.width - PADDING - scaledMaxPrimaryGunLength, verticalMiddle
				},
		resources.textWhiteBrush
	);
		
	resources.normalTextRenderer.draw(
		Utils::formatMoneyAmount(player.money, commonResources.configuration),
		rightSide
			? D2D1_RECT_F{PADDING, verticalMiddle, parentSize.width - PADDING - 40, parentSize.height}
			: D2D1_RECT_F{PADDING + 40, verticalMiddle, parentSize.width - PADDING, parentSize.height},
		resources.textGreenBrush
	);
	
	{
		const float activeTransitionValue = activeTransition.getValue();
		if (activeTransitionValue != 0) {
			winrt::com_ptr<ID2D1SolidColorBrush> activeOutlineBrush;
			renderTarget.CreateSolidColorBrush(
				{
					resources.activeOutlineColor.r, resources.activeOutlineColor.g,
					resources.activeOutlineColor.b, resources.activeOutlineColor.a * activeTransitionValue
				},
				activeOutlineBrush.put()
			);
			renderTarget.DrawRectangle(
				{-1, -1, parentSize.width+1, parentSize.height+1}, activeOutlineBrush.get(), 2
			);
		}
	}

	if (player.killsThisRound != 0) {
		const float killCountPos
			= rightSide ? -KILL_COUNT_OFFSET - KILL_COUNT_LENGTH : parentSize.width + KILL_COUNT_OFFSET;
		resources.normalTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		resources.normalTextRenderer.draw(
			std::to_wstring(player.killsThisRound),
			{killCountPos, 0, killCountPos + KILL_COUNT_ICON_POS, verticalMiddle},
			resources.textWhiteBrush
		);
		drawIcon(
			IconStorage::INDEX_DEAD,
			killCountPos + KILL_COUNT_ICON_POS, parentSize.height / 16, scale * 3 / 4,
			true, false, false, false
		);
		renderTarget.SetTransform(transform);
	}
	const auto oldMode = renderTarget.GetAntialiasMode();
	renderTarget.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	renderTarget.DrawSpriteBatch(
		spriteBatch.get(), commonResources.icons.getBitmap(),
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1_SPRITE_OPTIONS_NONE
	);
	renderTarget.SetAntialiasMode(oldMode);

	{
		const float statsTransitionValue = resources.statsTransition.getValue();
		if (statsTransitionValue != 0) {
			const bool statsTransiting = resources.statsTransition.transiting();
			const float currentLength = STATS_LENGTH * statsTransitionValue;
			if (statsTransiting) renderTarget.PushLayer(
				{
					rightSide
						? D2D1_RECT_F{-STAT_CELL_SPACING - currentLength, 0, -STAT_CELL_SPACING, parentSize.height}
						: D2D1_RECT_F{
							parentSize.width + STAT_CELL_SPACING, 0,
							parentSize.width + STAT_CELL_SPACING + currentLength, parentSize.height
						},
					nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
					1, nullptr, D2D1_LAYER_OPTIONS_NONE
				},
				statsLayer.get()
			);
			const float	
				start = rightSide
					? -STAT_CELL_SPACING - currentLength
					: parentSize.width + STAT_CELL_SPACING - STATS_LENGTH + currentLength,
				kdStart = rightSide ? start + MONEY_CELL_LENGTH + STAT_CELL_SPACING : start,
				kdEnd = kdStart + KD_CELL_LENGTH,
				moneyStart = rightSide ? start : start + KD_CELL_LENGTH + STAT_CELL_SPACING,
				moneyEnd = moneyStart + MONEY_CELL_LENGTH;
				
			renderTarget.FillRectangle({kdStart, 0, kdEnd, parentSize.height}, backgroundBrush.get());
			renderTarget.FillRectangle({moneyStart, 0, moneyEnd, parentSize.height}, backgroundBrush.get());
			resources.normalTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			resources.normalTextRenderer.draw(
				std::to_wstring(player.totalKills), {kdStart, 0, kdEnd, verticalMiddle}, resources.textWhiteBrush
			);
			resources.normalTextRenderer.draw(
				std::to_wstring(player.totalDeaths),
				{kdStart, verticalMiddle, kdEnd, parentSize.height},
				resources.textWhiteBrush
			);
			resources.normalTextRenderer.draw(
				L"-"s + Utils::formatMoneyAmount(
					std::min(0, player.startingMoney - player.money), commonResources.configuration
				),
				{moneyStart, 0, moneyEnd, verticalMiddle},
				resources.textWhiteBrush
			);
			resources.normalTextRenderer.draw(
				Utils::formatMoneyAmount(player.equipmentValue, commonResources.configuration),
				{moneyStart, verticalMiddle, moneyEnd, parentSize.height},
				resources.textWhiteBrush
			);
			if (statsTransiting) renderTarget.PopLayer();
		}
	}

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud