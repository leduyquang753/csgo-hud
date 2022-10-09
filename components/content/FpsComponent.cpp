#include "pch.h"

#include <string>

#include "components/base/Component.h"
#include "resources/CommonResources.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "utils/CommonConstants.h"

#include "components/content/FpsComponent.h"

using namespace std::string_literals;

namespace CsgoHud {

// == FpsComponent ==

FpsComponent::FpsComponent(CommonResources &commonResources): Component(commonResources) {
	commonResources.renderTarget->CreateSolidColorBrush({1, 1, 1, 1}, textBrush.put());
	
	winrt::com_ptr<IDWriteTextFormat> textFormat;
	commonResources.writeFactory->CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		14, L"", textFormat.put()
	);
	textRenderer.emplace(
		commonResources, textFormat,
		CommonConstants::FONT_OFFSET_RATIO, CommonConstants::FONT_LINE_HEIGHT_RATIO
	);
	
	commonResources.eventBus.listenToKeyEvent('F', [this](){ onVisibilityToggle(); });
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