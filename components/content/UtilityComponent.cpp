#include <string>
#include <string_view>

#include "pch.h"

#include "components/base/Component.h"
#include "data/IconStorage.h"
#include "data/WeaponTypes.h"
#include "movement/TransitionedValue.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"

#include "components/content/UtilityComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == UtilityComponent ==

UtilityComponent::UtilityComponent(
	CommonResources &commonResources, const bool rightSide, const TransitionedValue &transition
):
	Component(commonResources), rightSide(rightSide), transition(transition),
	normalColor{1, 1, 1, 1}, grayedColor{1, 1, 1, 0.5f}
{
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.CreateSolidColorBrush({0.35f, 0.72f, 0.96f, 1}, ctBrush.put());
	renderTarget.CreateSolidColorBrush({0.94f, 0.79f, 0.25f, 1}, tBrush.put());
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.5f}, headerShadeBrush.put());
	renderTarget.CreateSolidColorBrush({0, 0, 0, 0.3f}, backgroundBrush.put());
	renderTarget.CreateSolidColorBrush({1, 1, 1, 0.2f}, backgroundShadeBrush.put());
	renderTarget.CreateSolidColorBrush(normalColor, normalTextBrush.put());
	renderTarget.CreateSolidColorBrush(grayedColor, grayedTextBrush.put());
	renderTarget.CreateLayer(layer.put());
	
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	const auto fontFamily = commonResources.configuration.fontFamily.c_str();
	commonResources.writeFactory->CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	titleTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	textFormat = nullptr;
	commonResources.writeFactory->CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	totalTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	textFormat = nullptr;
	commonResources.writeFactory->CreateTextFormat(
		fontFamily, nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
	countTextRenderer.emplace(
		commonResources, textFormat, CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
}

void UtilityComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float transitionValue = transition.getValue();
	if (transitionValue == 0) return;

	int fragCount = 0, stunCount = 0, incendiaryCount = 0, smokeCount = 0, decoyCount = 0, totalCount = 0;
	for (int i = 0; i != 10; ++i) {
		const auto &player = commonResources.players[i];
		if (!player || player->team != ct) continue;
		for (const auto &grenade : player->grenades) if (grenade) {
			switch (*grenade) {
				case WeaponTypes::ID_FRAG_GRENADE:
					++fragCount;
					break;
				case WeaponTypes::ID_STUN_GRENADE:
					++stunCount;
					break;
				case WeaponTypes::ID_INCENDIARY_GRENADE:
				case WeaponTypes::ID_MOLOTOV_COCKTAIL:
					++incendiaryCount;
					break;
				case WeaponTypes::ID_SMOKE_GRENADE:
					++smokeCount;
					break;
				case WeaponTypes::ID_DECOY_GRENADE:
					++decoyCount;
					break;
			}
			++totalCount;
		}
	}
	
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	const bool transiting = transition.transiting();
	if (transiting) {
		renderTarget.PushLayer(
			{
				{0, 0, parentSize.width, parentSize.height},
				nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::Matrix3x2F::Identity(),
				1, nullptr,
				D2D1_LAYER_OPTIONS_NONE
			},
			layer.get()
		);
		renderTarget.SetTransform(
			D2D1::Matrix3x2F::Translation(parentSize.width * (1 - transitionValue) * (rightSide ? 1 : -1), 0)
			* transform
		);
	}

	const float
		verticalMiddle = parentSize.height / 2,
		contentMiddle = parentSize.height * 3/4,
		entryPadding = parentSize.width / 5 / 5;
	renderTarget.FillRectangle({0, 0, parentSize.width, verticalMiddle}, ct ? ctBrush.get() : tBrush.get());
	renderTarget.FillRectangle({0, verticalMiddle, parentSize.width, parentSize.height}, backgroundBrush.get());
	renderTarget.FillRectangle(
		{parentSize.width * totalCount / 20, 0, parentSize.width, verticalMiddle}, headerShadeBrush.get()
	);
	titleTextRenderer->draw(L"UTILITY"sv, {8, 0, parentSize.width - 8, verticalMiddle}, normalTextBrush);
	totalTextRenderer->draw(
		std::to_wstring(totalCount) + L" / 20"s, {8, 0, parentSize.width - 8, verticalMiddle}, normalTextBrush
	);
	
	winrt::com_ptr<ID2D1SpriteBatch> spriteBatch;
	renderTarget.CreateSpriteBatch(spriteBatch.put());
	auto drawCount = [this, &parentSize, &renderTarget, verticalMiddle, contentMiddle, entryPadding, &spriteBatch](
		const float startX, const int iconIndex, const int count, const int maxCount
	) {
		renderTarget.FillRectangle(
			{
				startX, parentSize.height * (1 - 0.5f * count / maxCount),
				startX + parentSize.width/5, parentSize.height
			},
			backgroundShadeBrush.get()
		);
		
		const auto &icon = commonResources.icons[iconIndex];
		const float iconWidth = 16.f * icon.width / icon.height;
		const D2D1_RECT_F destinationRect = {
			startX + entryPadding, contentMiddle - 8,
			startX + entryPadding + iconWidth, contentMiddle + 8
		};
		spriteBatch->AddSprites(
			1, &destinationRect, &icon.bounds, count == 0 ? &grayedColor : &normalColor, nullptr, 0, 0, 0, 0
		);

		countTextRenderer->draw(
			std::to_wstring(count),
			{startX, verticalMiddle, startX + parentSize.width/5 - entryPadding, parentSize.height},
			count == 0 ? grayedTextBrush : normalTextBrush
		);
	};
	drawCount(0, IconStorage::INDEX_FRAG_GRENADE, fragCount, 5);
	drawCount(parentSize.width / 5, IconStorage::INDEX_STUN_GRENADE, stunCount, 10);
	drawCount(
		parentSize.width * 2/5,
		ct ? IconStorage::INDEX_INCENDIARY_GRENADE : IconStorage::INDEX_MOLOTOV_COCKTAIL,
		incendiaryCount, 5
	);
	drawCount(parentSize.width * 3/5, IconStorage::INDEX_SMOKE_GRENADE, smokeCount, 5);
	drawCount(parentSize.width * 4/5, IconStorage::INDEX_DECOY_GRENADE, decoyCount, 5);
	
	const auto oldMode = renderTarget.GetAntialiasMode();
	renderTarget.SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	renderTarget.DrawSpriteBatch(
		spriteBatch.get(), commonResources.icons.getBitmap(),
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1_SPRITE_OPTIONS_NONE
	);
	renderTarget.SetAntialiasMode(oldMode);

	if (transiting) renderTarget.PopLayer();

	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud