#include "pch.h"

#include <string>

#include "components/base/Component.h"
#include "resources/CommonResources.h"

#include "components/content/TestComponent.h"

namespace CsgoHud {

// == TestComponent ==

TestComponent::TestComponent(CommonResources &commonResources):
	Component(commonResources),
	linear({{{60, 50}, {120, 500}, {120, 300}, {180, 1050}, {240, 50}}}),
	cubicBezier({{
		{60, 50}, {0.2, 0.4}, {0.8, 1.5},
		{120, 500}, {0, 0}, {0, 0},
		{120, 300}, {0.3, 0}, {0.7, 1},
		{180, 1050}, {0.5, 0}, {1, 1},
		{240, 50}
	}})
{
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
	renderTarget.FillRectangle({0, 50, linear.getValue(static_cast<float>(count)), 100}, brush.get());
	renderTarget.FillRectangle({0, 150, cubicBezier.getValue(static_cast<float>(count)), 200}, brush.get());
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud