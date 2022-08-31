#include "pch.h"

#include <string>

#include "components/base/Component.h"
#include "resources/CommonResources.h"

#include "components/content/TestComponent.h"

namespace CsgoHud {

// == TestComponent ==

TestComponent::TestComponent(CommonResources &commonResources): Component(commonResources) {
	commonResources.renderTarget->CreateSolidColorBrush({0, 0, 0, 1}, brush.put());
	commonResources.writeFactory->CreateTextFormat(
		L"Stratum2", nullptr,
		DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		36, L"", textFormat.put()
	);
}

void TestComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	const std::wstring countString = std::to_wstring(++count);
	renderTarget.DrawText(
		countString.c_str(), static_cast<UINT32>(countString.size()),
		textFormat.get(),
		{0, 0, parentSize.width, parentSize.height},
		brush.get(),
		D2D1_DRAW_TEXT_OPTIONS_NONE,
		DWRITE_MEASURING_MODE_NATURAL
	);
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud