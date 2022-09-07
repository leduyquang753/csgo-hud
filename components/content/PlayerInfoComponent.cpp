#include <string>

#include "pch.h"

#include "components/base/Component.h"
#include "data/IconStorage.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/LinearMovementFunction.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/CommonConstants.h"

#include "components/content/PlayerInfoComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == PlayerInfoComponent ==

PlayerInfoComponent::PlayerInfoComponent(
	CommonResources &commonResources,
	const D2D1_COLOR_F &backgroundInactiveColor,
	const D2D1_COLOR_F &backgroundActiveColor,
	const D2D1_COLOR_F &activeOutlineColor,
	const winrt::com_ptr<ID2D1SolidColorBrush> &teamCtBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &teamTBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &healthBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textWhiteBrush,
	const winrt::com_ptr<ID2D1SolidColorBrush> &textGreenBrush,
	const winrt::com_ptr<IDWriteTextFormat> &normalTextFormat,
	const winrt::com_ptr<IDWriteTextFormat> &boldTextFormat,
	FixedWidthDigitTextRenderer &normalTextRenderer,
	FixedWidthDigitTextRenderer &boldTextRenderer
):
	Component(commonResources),
	backgroundInactiveColor(backgroundInactiveColor), backgroundActiveColor(backgroundActiveColor),
	activeOutlineColor(activeOutlineColor),
	teamCtBrush(teamCtBrush), teamTBrush(teamTBrush), healthBrush(healthBrush),
	textWhiteBrush(textWhiteBrush), textGreenBrush(textGreenBrush),
	normalTextFormat(normalTextFormat), boldTextFormat(boldTextFormat),
	normalTextRenderer(normalTextRenderer), boldTextRenderer(boldTextRenderer),
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
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateEffect(CLSID_D2D1Tint, inactiveEffect.put());
	inactiveEffect->SetValue(D2D1_TINT_PROP_COLOR, D2D1_VECTOR_4F{1, 1, 1, 0.6f});
	renderTarget.CreateEffect(CLSID_D2D1Tint, emptyActiveEffect.put());
	emptyActiveEffect->SetValue(D2D1_TINT_PROP_COLOR, D2D1_VECTOR_4F{1, 0.5f, 0.5f, 1});
	renderTarget.CreateEffect(CLSID_D2D1Tint, emptyInactiveEffect.put());
	emptyInactiveEffect->SetValue(D2D1_TINT_PROP_COLOR, D2D1_VECTOR_4F{1, 0.5f, 0.5f, 0.6f});

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
		PADDING = 4;

	if (index != lastIndex) {
		lastIndex = index;
		oldHealth = 0;
		healthDecayTime = 0;
		healthTransition.setValue(0);
		wasActive = false;
		activeTransition.setValue(0);
	}
	if (active != wasActive) {
		wasActive = active;
		activeTransition.transition(active ? 1.f : 0.f);
	}
	
	if (index == -1) return;
	const auto &optionalPlayer = commonResources.players[index];
	if (!optionalPlayer) return;
	const auto &player = *optionalPlayer;
		
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	
	const bool rightSide = index > 4;
	const float
		verticalMiddle = parentSize.height / 2,
		scale = verticalMiddle / CommonConstants::ICON_HEIGHT,
		scaledMaxPrimaryGunLength = MAX_PRIMARY_GUN_LENGTH * scale;
	std::wstring textToRender;

	auto drawIcon = [this, &transform, &renderTarget](
		const int index, const float x, const float y, const float scale,
		const bool active, const bool empty, const bool rightSide, const bool flipped
	) {
		const auto &icon = commonResources.icons[index];
		auto iconTransform
			= D2D1::Matrix3x2F::Scale(scale, scale, {0, 0})
			* D2D1::Matrix3x2F::Translation(rightSide ? x - icon.width * scale : x, y)
			* transform;
		if (flipped) iconTransform = D2D1::Matrix3x2F::Scale(-1, 1, {icon.width / 2.f, 0}) * iconTransform;
		renderTarget.SetTransform(iconTransform);
		if (active && !empty) {
			renderTarget.DrawImage(
				icon.source.get(), nullptr, nullptr,
				D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, D2D1_COMPOSITE_MODE_SOURCE_OVER
			);
		} else {
			auto effect = empty
				? active ? emptyActiveEffect.get() : emptyInactiveEffect.get()
				: inactiveEffect.get();
			effect->SetInputEffect(0, icon.source.get());
			renderTarget.DrawImage(effect);
		}
	};
	
	{
		winrt::com_ptr<ID2D1SolidColorBrush> backgroundBrush;
		const float in = activeTransition.getValue(), out = 1 - in;
		renderTarget.CreateSolidColorBrush(
			{
				backgroundInactiveColor.r * out + backgroundActiveColor.r * in,
				backgroundInactiveColor.g * out + backgroundActiveColor.g * in,
				backgroundInactiveColor.b * out + backgroundActiveColor.b * in,
				backgroundInactiveColor.a * out + backgroundActiveColor.a * in
			},
			backgroundBrush.put()
		);
		renderTarget.FillRectangle({0, 0, parentSize.width, parentSize.height}, backgroundBrush.get());
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
				healthBrush.get()
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
			player.team ? teamCtBrush.get() : teamTBrush.get()
		);
		boldTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		boldTextRenderer.draw(
			std::to_wstring(player.health),
			rightSide
				? D2D1_RECT_F{
					parentSize.width - PADDING - HEALTH_LENGTH, 0, parentSize.width - PADDING, verticalMiddle
				}
				: D2D1_RECT_F{PADDING, 0, PADDING + HEALTH_LENGTH - 4, verticalMiddle},
			textWhiteBrush
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
				player.activeSlot == PlayerData::SLOT_ZEUS, false, !rightSide, false
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
			true, false, rightSide, false
		);
	}
	renderTarget.SetTransform(transform);
	
	normalTextFormat->SetTextAlignment(rightSide ? DWRITE_TEXT_ALIGNMENT_LEADING : DWRITE_TEXT_ALIGNMENT_TRAILING);
	normalTextRenderer.draw(
		rightSide ? L"| "s + std::to_wstring(index+1) : std::to_wstring(index+1) + L" |"s,
		rightSide
			? D2D1_RECT_F{
				parentSize.width - PADDING - HEALTH_LENGTH - SLOT_RENDER_LENGTH, 0, parentSize.width, verticalMiddle
			}
			: D2D1_RECT_F{
				0, 0, PADDING + HEALTH_LENGTH + SLOT_RENDER_LENGTH, verticalMiddle
			},
		textWhiteBrush
	);
	normalTextFormat->SetTextAlignment(rightSide ? DWRITE_TEXT_ALIGNMENT_TRAILING : DWRITE_TEXT_ALIGNMENT_LEADING);
	normalTextRenderer.draw(
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
		textWhiteBrush
	);
		
	normalTextRenderer.draw(
		std::to_wstring(player.money) + L" $",
		rightSide
			? D2D1_RECT_F{PADDING, verticalMiddle, parentSize.width - PADDING - 40, parentSize.height}
			: D2D1_RECT_F{PADDING + 40, verticalMiddle, parentSize.width - PADDING, parentSize.height},
		textGreenBrush
	);
	
	{
		const float activeTransitionValue = activeTransition.getValue();
		if (activeTransitionValue != 0) {
			winrt::com_ptr<ID2D1SolidColorBrush> activeOutlineBrush;
			renderTarget.CreateSolidColorBrush(
				{
					activeOutlineColor.r, activeOutlineColor.g,
					activeOutlineColor.b, activeOutlineColor.a * activeTransitionValue
				},
				activeOutlineBrush.put()
			);
			renderTarget.DrawRectangle(
				{-1, -1, parentSize.width+1, parentSize.height+1}, activeOutlineBrush.get(), 2
			);
		}
	}

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud