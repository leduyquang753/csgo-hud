#include "pch.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numbers>
#include <string>
#include <string_view>
#include <vector>

#include "components/base/Component.h"
#include "data/BombData.h"
#include "data/IconStorage.h"
#include "data/PlayerData.h"
#include "resources/CommonResources.h"
#include "text/NormalTextRenderer.h"
#include "utils/CommonConstants.h"

#include "components/content/MinimapComponent.h"

using namespace std::string_view_literals;

namespace CsgoHud {

// == MinimapComponent ==

MinimapComponent::MinimapComponent(CommonResources &commonResources):
	Component(commonResources),
	droppedBombColor{0.94f, 0.79f, 0.25f, 1},
	plantedBombColor{1, 0, 0, 1},
	defusedBombColor{0, 1, 0, 1},
	ctColor{0.35f, 0.72f, 0.96f, 1},
	tColor{0.94f, 0.79f, 0.25f, 1},
	neutralColor{0.8f, 0.8f, 0.8f, 1},
	bombRedFlashColor{1, 0.6f, 0.6f, 1},
	bombWhiteFlashColor{1, 1, 1, 1},
	explosionInnerAnimation{{{{0, 0}, {0, 0.25f}, {0.25f, 1}, {4000, 1}}}},
	explosionOuterAnimation{{{{0, 0}, {0, 1}, {0.1f, 0.9f}, {4000, 1}}}}
{
	float currentFlashTime = 40000;
	while (currentFlashTime > 1000) {
		flashTimes.push_back(static_cast<int>(currentFlashTime));
		currentFlashTime -= std::max(150.f, 100.f + 900.f * currentFlashTime / 40000);
	}
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, whiteBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.7f}, bombsiteNameBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0, 0, 1}, bombBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, flashBrush.put());
	renderTarget.CreateSolidColorBrush({0.8f, 0.8f, 0.8f, 1}, smokeBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.5f, 0.5f, 1}, fireBrush.put());
	
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		12, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	numberRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	textFormat = nullptr;
	writeFactory.CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		16, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	bombsiteNameRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);

	renderTarget.CreateLayer(layer1.put());
	renderTarget.CreateLayer(layer2.put());	
}

void MinimapComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const auto &map = commonResources.map;
	
	if (!map.mapAvailable) return;
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);

	// Draw the map image.
	const float
		imageScale = parentSize.width / map.mapWidth,
		effectiveScale = map.mapScale / imageScale;
	renderTarget.DrawBitmap(
		map.mapImage.get(), {0, 0, parentSize.width, map.mapHeight * imageScale}, 1,
		D2D1_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR, nullptr, nullptr
	);

	const float
		lowerLevelOffsetX = map.hasLowerLevel ? map.lowerLevelOffsetX * imageScale : 0,
		lowerLevelOffsetY = map.hasLowerLevel ? map.lowerLevelOffsetY * imageScale : 0;

	// Draw bombsite names.
	auto drawBombsiteName = [
		this, &renderTarget, &map, imageScale, effectiveScale, lowerLevelOffsetX, lowerLevelOffsetY
	](std::wstring_view name, const D2D1_VECTOR_3F &position) {
		float
			x = (position.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - position.y) / effectiveScale,
			z = position.z;
		if (map.hasLowerLevel && z < map.levelSeparationHeight) {
			x += lowerLevelOffsetX;
			y += lowerLevelOffsetY;
		}
		bombsiteNameRenderer->draw(name, {x - 10, y - 10, x + 10, y + 10}, bombsiteNameBrush);
	};
	drawBombsiteName(L"A"sv, map.bombsiteACenter);
	drawBombsiteName(L"B"sv, map.bombsiteBCenter);

	// Draw the players and the bomb.
	winrt::com_ptr<ID2D1SpriteBatch> spriteBatch;
	renderTarget.CreateSpriteBatch(spriteBatch.put());
	auto drawIcon = [this, &spriteBatch](
		const int index, const float x, const float y, const float halfSize, const D2D1_COLOR_F &color
	) {
		const auto &icon = commonResources.icons[index];
		const float halfWidth = halfSize * icon.width / icon.height;
		const D2D1_RECT_F destinationRect
			= {x - halfWidth, y - halfSize, x + halfWidth, y + halfSize};
		spriteBatch->AddSprites(1, &destinationRect, &icon.bounds, &color, nullptr, 0, 0, 0, 0);
	};
	auto commitIconDraws = [this, &renderTarget, &spriteBatch] {
		const auto oldMode = renderTarget.GetAntialiasMode();
		renderTarget.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
		renderTarget.DrawSpriteBatch(
			spriteBatch.get(), commonResources.icons.getBitmap(),
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			D2D1_SPRITE_OPTIONS_NONE
		);
		renderTarget.SetAntialiasMode(oldMode);
	};
	auto drawPlayerMarkerComponent = [this, &renderTarget](
		const float x, const float y, const float angle, const bool big,
		const winrt::com_ptr<ID2D1SolidColorBrush> &brush
	) {
		const float radius = big ? 10.f : 8.f;
		renderTarget.FillEllipse({x, y, radius, radius}, brush.get());
		winrt::com_ptr<ID2D1PathGeometry> path;
		commonResources.d2dFactory->CreatePathGeometry(path.put());
		winrt::com_ptr<ID2D1GeometrySink> sink;
		path->Open(sink.put());
		auto computePoint = [x, y](const float radius, const float angle) {
			// Gotta flip the Y offset as the screen XY is different from the game XY.
			return D2D1_POINT_2F{x + radius * std::cos(angle), y - radius * std::sin(angle)};
		};
		sink->BeginFigure(
			computePoint(radius, angle + std::numbers::pi_v<float>/4), // Left.
			D2D1_FIGURE_BEGIN_FILLED
		);
		sink->AddLine(computePoint(radius * std::numbers::sqrt2_v<float>, angle)); // Center.
		sink->AddLine(computePoint(radius, angle - std::numbers::pi_v<float>/4)); // Right.
		sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		sink->Close();
		renderTarget.FillGeometry(path.get(), brush.get());
	};
	auto drawPlayerEffect = [this, &renderTarget, &drawPlayerMarkerComponent](
		const float x, const float y, const float angle, const int amount, const float multiplier,
		const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1SolidColorBrush> &brush
	) {
		if (amount == 0) return;
		renderTarget.PushLayer(
			{
				bounds,
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				amount * multiplier / 255.f, nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			layer2.get()
		);
		drawPlayerMarkerComponent(x, y, angle, false, brush);
		renderTarget.PopLayer();
	};
	auto drawPlayerMarker = [
		this, &transform, &renderTarget, &drawIcon, &drawPlayerMarkerComponent, &drawPlayerEffect
	](
		const PlayerData &player,
		const float x, const float y, const float angle, const bool active, const float alpha, const int slot,
		const winrt::com_ptr<ID2D1SolidColorBrush> &brush, const float deathAlpha, const D2D1_COLOR_F &deathColor
	) {
		static const float LAYER_RANGE = 32;
		const D2D1_RECT_F bounds = {x - LAYER_RANGE, y - LAYER_RANGE, x + LAYER_RANGE, y + LAYER_RANGE};
		if (alpha != 1) renderTarget.PushLayer(
			{
				bounds,
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				alpha, nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			layer1.get()
		);
		if (deathAlpha < 0) {
			if (active) drawPlayerMarkerComponent(x, y, angle, true, whiteBrush);
			drawPlayerMarkerComponent(x, y, angle, false, brush);
			drawPlayerEffect(x, y, angle, player.flashAmount, 0.6f, bounds, flashBrush);
			drawPlayerEffect(x, y, angle, player.smokeAmount, 1, bounds, smokeBrush);
			drawPlayerEffect(x, y, angle, player.fireAmount, 0.6f, bounds, fireBrush);
			numberRenderer->draw(std::to_wstring(slot+1), bounds, whiteBrush);
		} else {
			D2D1_COLOR_F color = {
				deathColor.r, deathColor.g, deathColor.b, deathColor.a * deathAlpha
			};
			drawIcon(IconStorage::INDEX_X, x, y, 6, color);
		}
		if (alpha != 1) renderTarget.PopLayer();
	};
	auto drawPlayerMarkerSet = [
		this, &map, &renderTarget, effectiveScale, &drawPlayerMarker, lowerLevelOffsetX, lowerLevelOffsetY
	](const int slot, const bool active) {
		const auto &player = *commonResources.players[slot];
		const float
			x = (player.position.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - player.position.y) / effectiveScale,
			z = player.position.z,
			angle = player.forward.x == 0 && player.forward.y == 0
				? static_cast<float>(-std::numbers::pi_v<float>)
				: std::atan2(player.forward.y, player.forward.x);
		const auto &brush = player.team
			? ctBrush
			: player.hasC4OrDefuseKit ? bombBrush : tBrush;
		const auto &deathColor = player.team ? ctColor : tColor;
		float deathAlpha = -1;
		if (player.health == 0) {
			const int timePassed = commonResources.time - player.lastDeathTime;
			deathAlpha
				= timePassed < 3000 ? 1.f
				: timePassed < 5000 ? (5000 - timePassed) * 0.5f / 2000.f + 0.5f
				: 0.5f;
		}
		if (map.hasLowerLevel) {
			if (z >= map.levelSeparationTop) {
				drawPlayerMarker(player, x, y, angle, active, 1, slot, brush, deathAlpha, deathColor);
			} else if (z > map.levelSeparationBottom) {
				const float topAlpha = (z - map.levelSeparationBottom) / map.levelTransitionRange;
				drawPlayerMarker(player, x, y, angle, active, topAlpha, slot, brush, deathAlpha, deathColor);
				drawPlayerMarker(
					player, x + lowerLevelOffsetX, y + lowerLevelOffsetY,
					angle, active, 1 - topAlpha, slot, brush, deathAlpha, deathColor
				);
			} else {
				drawPlayerMarker(
					player, x + lowerLevelOffsetX, y + lowerLevelOffsetY,
					angle, active, 1, slot, brush, deathAlpha, deathColor
				);
			}
		} else {
			drawPlayerMarker(player, x, y, angle, active, 1, slot, brush, deathAlpha, deathColor);
		}
	};
	const auto &players = commonResources.players;
	int activePlayerIndex = players.getActivePlayerIndex();
	if (activePlayerIndex != -1) {
		const auto &activePlayer = players[activePlayerIndex];
		if (!activePlayer || activePlayer->health == 0) activePlayerIndex = -1;
	}
	int bombCarrierIndex = -1;
	// Draw dead players first.
	for (int i = 0; i != 10; ++i) {
		const auto &player = players[i];
		if (player && player->health == 0) drawPlayerMarkerSet(i, false);
	}

	// Draw underlaying grenade effects.
	auto drawGrenadeEffect = [this, &renderTarget](
		const float x, const float y, D2D1_COLOR_F color, const float alpha, const float radius
	) {
		color.a *= alpha;
		winrt::com_ptr<ID2D1SolidColorBrush> brush;
		renderTarget.CreateSolidColorBrush(color, brush.put());
		renderTarget.FillEllipse({{x, y}, radius, radius}, brush.get());
	};
	auto drawGrenadeEffects = [&map, effectiveScale, lowerLevelOffsetX, lowerLevelOffsetY, &drawGrenadeEffect](
		const D2D1_VECTOR_3F &position, const D2D1_COLOR_F &color, const float alpha, const float radius
	) {
		const float
			x = (position.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - position.y) / effectiveScale,
			z = position.z;
		if (map.hasLowerLevel) {
			if (z >= map.levelSeparationTop) {
				drawGrenadeEffect(x, y, color, alpha, radius);
			} else if (z > map.levelSeparationBottom) {
				const float alphaMultiplier = (z - map.levelSeparationBottom) / map.levelTransitionRange;
				drawGrenadeEffect(x, y, color, alpha * alphaMultiplier, radius);
				drawGrenadeEffect(
					x + lowerLevelOffsetX, y + lowerLevelOffsetY, color, alpha * (1-alphaMultiplier), radius
				);
			} else {
				drawGrenadeEffect(x + lowerLevelOffsetX, y + lowerLevelOffsetY, color, alpha, radius);
			}
		} else {
			drawGrenadeEffect(x, y, color, alpha, radius);
		}
	};
	const auto &grenades = commonResources.grenades;
	for (const auto &entry : grenades.burningAreas) {
		const auto &area = entry.second;
		if (area.burningTime >= (area.extinguished ? 500 : 7000)) continue;
		const float alpha = area.extinguished
			? (500 - area.burningTime) / 500.f
			: area.burningTime > 6000 ? (7000 - area.burningTime) / 1000.f : 1;
		for (const auto &subentry : area.pieceMap) {
			const auto &piece = subentry.second;
			drawGrenadeEffects(
				piece.position, {1, 0.6f, 0.35f, 0.3f},
				(piece.burningTime < 500 ? piece.burningTime / 500.f : 1) * alpha, 80 / effectiveScale
			);
		}
	}
	
	const auto &bomb = commonResources.bomb;
	if (
		bomb.bombState != BombData::State::CARRIED
		&& bomb.bombState != BombData::State::PLANTING
		&& bomb.bombState != BombData::State::EXPLODED
	) {
		const float
			x = (bomb.bombPosition.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - bomb.bombPosition.y) / effectiveScale,
			z = bomb.bombPosition.z;
		auto color
			= bomb.bombState == BombData::State::DROPPED ? droppedBombColor
			: bomb.bombState == BombData::State::PLANTED
				|| bomb.bombState == BombData::State::DEFUSING
				|| bomb.bombState == BombData::State::DETONATING
				? plantedBombColor
			: defusedBombColor;
		const bool shouldDrawFlash
			= bomb.bombState == BombData::State::PLANTED || bomb.bombState == BombData::State::DEFUSING
				? (
					bomb.bombTimeLeft < 1000
					|| std::ranges::upper_bound(
						flashTimes, std::min(40000, bomb.bombTimeLeft), std::ranges::greater()
					)[-1] - bomb.bombTimeLeft < 125
				)
				: (
					bomb.bombState == BombData::State::DETONATING
					&& bomb.bombTimeLeft * 22 / 1000 % 2 == 1
				);
		const auto flashColor
			= bomb.bombState == BombData::State::DETONATING || bomb.bombTimeLeft < 1000
				? bombWhiteFlashColor : bombRedFlashColor;
		auto drawBomb = [
			&transform, &renderTarget, &spriteBatch, &drawIcon, &commitIconDraws, &color,
			shouldDrawFlash, flashColor
		](const float x, const float y, const D2D1_COLOR_F &color) {
			drawIcon(IconStorage::INDEX_C4, x, y, 8, color);
			if (shouldDrawFlash) drawIcon(IconStorage::INDEX_LED, x-2, y-1, 12, flashColor);
		};
		if (map.hasLowerLevel) {
			if (z >= map.levelSeparationTop) {
				drawBomb(x, y, color);
			} else if (z > map.levelSeparationBottom) {
				color.a = (z - map.levelSeparationBottom) / map.levelTransitionRange;
				drawBomb(x, y, color);
				drawBomb(x + lowerLevelOffsetX, y + lowerLevelOffsetY, color);
			} else {
				drawBomb(x + lowerLevelOffsetX, y + lowerLevelOffsetY, color);
			}
		} else {
			drawBomb(x, y, color);
		}
	}
	commitIconDraws();
	
	// Draw alive players on top.
	for (int i = 0; i != 10; ++i) {
		const auto &player = players[i];
		if (player && player->health != 0) {
			if (!player->team && player->hasC4OrDefuseKit) bombCarrierIndex = i;
			else if (i != activePlayerIndex) drawPlayerMarkerSet(i, false);
		}
	}
	// Put the bomb carrier then the active player to the top for ease of viewing.
	if (bombCarrierIndex != -1 && bombCarrierIndex != activePlayerIndex)
		drawPlayerMarkerSet(bombCarrierIndex, false);
	if (activePlayerIndex != -1)
		drawPlayerMarkerSet(activePlayerIndex, true);

	// Draw thrown grenades.
	auto drawThrownGrenade = [this, &map, effectiveScale, lowerLevelOffsetX, lowerLevelOffsetY, &drawIcon](
		const GrenadesData::Grenade &grenade, const int iconIndex
	) {
		auto color = grenade.throwerFound
			? grenade.throwerTeam ? ctColor : tColor
			: neutralColor;
		const float
			x = (grenade.position.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - grenade.position.y) / effectiveScale,
			z = grenade.position.z;
		if (map.hasLowerLevel) {
			if (z >= map.levelSeparationTop) {
				drawIcon(iconIndex, x, y, 8, color);
			} else if (z > map.levelSeparationBottom) {
				color.a = (z - map.levelSeparationBottom) / map.levelTransitionRange;
				drawIcon(iconIndex, x, y, 8, color);
				color.a = 1 - color.a;
				drawIcon(iconIndex, x + lowerLevelOffsetX, y + lowerLevelOffsetY, 8, color);
			} else {
				drawIcon(iconIndex, x + lowerLevelOffsetX, y + lowerLevelOffsetY, 8, color);
			}
		} else {
			drawIcon(iconIndex, x, y, 8, color);
		}
	};
	spriteBatch->Clear();
	for (const auto &[id, grenade] : grenades.fragGrenades)
		if (grenade.explodingTime == -1) drawThrownGrenade(grenade, IconStorage::INDEX_FRAG_GRENADE);
	for (const auto &[id, grenade] : grenades.stunGrenades)
		drawThrownGrenade(grenade, IconStorage::INDEX_STUN_GRENADE);
	for (const auto &[id, grenade] : grenades.smokeGrenades)
		if (grenade.smokingTime == 0) drawThrownGrenade(grenade, IconStorage::INDEX_SMOKE_GRENADE);
	for (const auto &[id, grenade] : grenades.incendiaryGrenades) {
		drawThrownGrenade(
			grenade,
			!grenade.throwerFound || grenade.throwerTeam
				? IconStorage::INDEX_INCENDIARY_GRENADE : IconStorage::INDEX_MOLOTOV_COCKTAIL
		);
	}
	for (const auto &[id, grenade] : grenades.decoyGrenades)
		drawThrownGrenade(grenade, IconStorage::INDEX_DECOY_GRENADE);
	commitIconDraws();

	// Draw overlaying grenade effects.
	for (const auto &entry : grenades.fragGrenades) {
		const auto &grenade = entry.second;
		if (grenade.explodingTime != -1 && grenade.explodingTime < 4000) {
			const float animationValue = grenade.explodingTime / 4000.f;
			drawGrenadeEffects(
				grenade.position, {0, 0, 0, 0.8f},
				1 - animationValue, 384 / effectiveScale * (1 - std::pow(animationValue, 3.f))
			);
		}
	}
	auto drawExplodedGrenades = [&drawGrenadeEffects](
		const std::vector<GrenadesData::ExplodedGrenade> &grenades, const D2D1_COLOR_F &color,
		const float startingRadius, const int lifetime
	) {
		for (const auto &grenade : grenades) {
			const float animationValue = static_cast<float>(grenade.animationTime) / lifetime;
			drawGrenadeEffects(
				grenade.position, color,
				1 - animationValue, startingRadius * (1 - std::pow(animationValue, 3.f))
			);
		}
	};
	drawExplodedGrenades(grenades.explodedDecoyGrenades, {0, 0, 0, 0.4f}, 8, 4000);
	drawExplodedGrenades(grenades.explodedStunGrenades, {1, 1, 1, 0.8f}, 20, 250);
	for (const auto &entry : grenades.smokeGrenades) {
		const auto &grenade = entry.second;
		if (grenade.smokingTime < 18000) {
			drawGrenadeEffects(
				grenade.position, {0.8f, 0.8f, 0.8f, 0.7f},
				grenade.smokingTime < 750 ? grenade.smokingTime / 750.f
				: grenade.smokingTime > 16000 ? (18000 - grenade.smokingTime) / 2000.f
				: 1,
				144 / effectiveScale * (grenade.smokingTime < 300 ? grenade.smokingTime / 300.f : 1)
			);
		}
	}

	// Draw the bomb explision.
	if (bomb.bombState == BombData::State::EXPLODED && bomb.bombTimeLeft < 4000) {
		float
			x = (bomb.bombPosition.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - bomb.bombPosition.y) / effectiveScale;
		auto drawEllipse = [&renderTarget, &bomb](
			const float x, const float y, const float maxRadius, const float animationValue
		) {
			winrt::com_ptr<ID2D1SolidColorBrush> brush;
			renderTarget.CreateSolidColorBrush({1, 1, 1, 1 - animationValue}, brush.put());
			const float radius = maxRadius * animationValue;
			renderTarget.FillEllipse({{x, y}, radius, radius}, brush.get());
		};
		// Always draw the explosion on both levels as the bomb doesn't affect damage only on the level it is in.
		drawEllipse(x, y, 60, explosionOuterAnimation.getValue(static_cast<float>(bomb.bombTimeLeft)));
		drawEllipse(x, y, 50, explosionInnerAnimation.getValue(static_cast<float>(bomb.bombTimeLeft)));
		if (map.hasLowerLevel) {
			x += lowerLevelOffsetX;
			y += lowerLevelOffsetY;
			drawEllipse(x, y, 60, explosionOuterAnimation.getValue(static_cast<float>(bomb.bombTimeLeft)));
			drawEllipse(x, y, 50, explosionInnerAnimation.getValue(static_cast<float>(bomb.bombTimeLeft)));
		}
	}

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud