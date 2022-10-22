#include "pch.h"

#include <string>
#include <string_view>

#include "components/base/Component.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/Utils.h"

#include "components/content/FpsComponent.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == FpsComponent ==

FpsComponent::FpsComponent(CommonResources &commonResources): Component(commonResources) {
	commonResources.renderTarget->CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	commonResources.writeFactory->CreateTextFormat(
		commonResources.configuration.fontFamily.c_str(), nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	textRenderer.emplace(
		commonResources, textFormat,
		commonResources.configuration.fontOffsetRatio, commonResources.configuration.fontLineHeightRatio
	);
	
	commonResources.eventBus.listenToKeyEvent(
		Utils::parseKeyCode(commonResources.configuration.keybindings["toggleFps"sv].value().get_string().value()),
		[this](){ onVisibilityToggle(); }
	);
}

void FpsComponent::onVisibilityToggle() {
	shown = !shown;
}

void FpsComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const int second = commonResources.time / 1000;
	if (second != currentSecond) {
		lastFps = second == currentSecond+1 ? fps : 0;
		currentSecond = second;
		fps = 0;
	} else {
		++fps;
	}

	if (!shown) return;
	commonResources.renderTarget->SetTransform(transform);
	textRenderer->draw(std::to_wstring(lastFps) + L" FPS"s, {0, 0, parentSize.width, 16}, textBrush);
	commonResources.renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud