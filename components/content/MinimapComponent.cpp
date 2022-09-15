#include <cmath>
#include <string>
#include <string_view>

#include "pch.h"

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
	tColor{0.94f, 0.79f, 0.25f, 1}
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, whiteBrush.put());
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 0.7f}, bombsiteNameBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0, 0, 1}, bombBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 1}, flashBrush.put());
	renderTarget.CreateSolidColorBrush({1, 0.5f, 0.5f, 1}, smokeBrush.put());
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

	// Draw bombsite names.
	auto drawBombsiteName = [this, &renderTarget, &map, imageScale, effectiveScale](
		std::wstring_view name, const D2D1_VECTOR_3F &position
	) {
		float
			x = (position.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - position.y) / effectiveScale,
			z = position.z;
		if (map.hasLowerLevel && z < map.levelSeparationHeight) {
			x += map.lowerLevelOffsetX * imageScale;
			y += map.lowerLevelOffsetY * imageScale;
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
		const D2D1_RECT_F destinationRect
			= {x - halfSize, y - halfSize, x + halfSize, y + halfSize};
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
		renderTarget.FillEllipse({{x, y}, radius, radius}, brush.get());
		winrt::com_ptr<ID2D1PathGeometry> path;
		commonResources.d2dFactory->CreatePathGeometry(path.put());
		winrt::com_ptr<ID2D1GeometrySink> sink;
		path->Open(sink.put());
		auto computePoint = [x, y](const float radius, const float angle) {
			// Gotta flip the Y offset as the screen XY is different from the game XY.
			return D2D1_POINT_2F{x + radius * std::cos(angle), y - radius * std::sin(angle)};
		};
		sink->BeginFigure(
			computePoint(radius, angle + static_cast<float>(M_PI_4)), // Left.
			D2D1_FIGURE_BEGIN_FILLED
		);
		sink->AddLine(computePoint(radius * static_cast<float>(M_SQRT2), angle)); // Center.
		sink->AddLine(computePoint(radius, angle - static_cast<float>(M_PI_4))); // Right.
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
	auto drawPlayerMarkerSet = [this, &map, &renderTarget, imageScale, effectiveScale, &drawPlayerMarker](
		const int slot, const bool active
	) {
		const auto &player = *commonResources.players[slot];
		const float
			x = (player.position.x - map.leftCoordinate) / effectiveScale,
			y = (map.topCoordinate - player.position.y) / effectiveScale,
			z = player.position.z,
			angle = player.forward.x == 0 && player.forward.y == 0
				? static_cast<float>(-M_PI_4)
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
					player, x + map.lowerLevelOffsetX * imageScale, y + map.lowerLevelOffsetY * imageScale,
					angle, active, 1 - topAlpha, slot, brush, deathAlpha, deathColor
				);
			} else {
				drawPlayerMarker(
					player, x + map.lowerLevelOffsetX * imageScale, y + map.lowerLevelOffsetY * imageScale,
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
		static const float BOMB_HALF_SIZE = 8;
		auto color
			= bomb.bombState == BombData::State::DROPPED ? droppedBombColor
			: bomb.bombState == BombData::State::PLANTED
				|| bomb.bombState == BombData::State::DEFUSING
				|| bomb.bombState == BombData::State::DETONATING
				? plantedBombColor
			: defusedBombColor;
		if (map.hasLowerLevel) {
			if (z >= map.levelSeparationTop) {
				drawIcon(IconStorage::INDEX_C4, x, y, BOMB_HALF_SIZE, color);
			} else if (z > map.levelSeparationBottom) {
				color.a = (z - map.levelSeparationBottom) / map.levelTransitionRange;
				drawIcon(IconStorage::INDEX_C4, x, y, BOMB_HALF_SIZE, color);
				drawIcon(
					IconStorage::INDEX_C4,
					x + map.lowerLevelOffsetX * imageScale, y + map.lowerLevelOffsetY * imageScale,
					BOMB_HALF_SIZE, color
				);
			} else {
				drawIcon(
					IconStorage::INDEX_C4,
					x + map.lowerLevelOffsetX * imageScale, y + map.lowerLevelOffsetY * imageScale,
					BOMB_HALF_SIZE, color
				);
			}
		} else {
			drawIcon(IconStorage::INDEX_C4, x, y, BOMB_HALF_SIZE, color);
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

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud