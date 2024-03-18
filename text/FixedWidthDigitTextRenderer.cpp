#include <algorithm>
#include <array>
#include <cstdlib>
#include <string_view>

#include "pch.h"

#include "resources/CommonResources.h"
#include "text/TextRenderer.h"

#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

// == FixedWidthDigitTextRenderer ==

FixedWidthDigitTextRenderer::FixedWidthDigitTextRenderer(
	CommonResources &commonResources, const winrt::com_ptr<IDWriteTextFormat> &textFormat,
	const float offsetRatio, const float lineHeightRatio
):
	TextRenderer(commonResources), textFormat(textFormat),
	lineSpacingSettings{
		.method = DWRITE_LINE_SPACING_METHOD_UNIFORM,
		.height = textFormat->GetFontSize(),
		.baseline = textFormat->GetFontSize(),
		.leadingBefore = 0,
		.fontLineGapUsage = DWRITE_FONT_LINE_GAP_USAGE_DISABLED
	},
	verticalOffset(textFormat->GetFontSize() * offsetRatio),
	lineHeight(textFormat->GetFontSize() * lineHeightRatio)
{
	std::array<wchar_t, 2> digitString = {};
	std::array<float, 10> digitWidths;
	DWRITE_TEXT_METRICS textMetrics;
	for (int i = 0; i != 10; ++i) {
		digitString[0] = L'0' + i;
		winrt::com_ptr<IDWriteTextLayout> textLayout;
		commonResources.writeFactory->CreateTextLayout(
			digitString.data(), 1, textFormat.get(), 0, 0, textLayout.put()
		);
		textLayout->GetMetrics(&textMetrics);
		digitWidths[i] = textMetrics.width;
	}
	const float maxDigitWidth = std::ranges::max(digitWidths);
	for (int i = 0; i != 10; ++i) digitSpacings[i] = (maxDigitWidth - digitWidths[i]) / 2;
}


winrt::com_ptr<IDWriteTextLayout3> FixedWidthDigitTextRenderer::prepareLayout(
	std::wstring_view text, const D2D1_RECT_F &bounds
) const {
	winrt::com_ptr<IDWriteTextLayout> textLayout;
	commonResources.writeFactory->CreateTextLayout(
		text.data(), static_cast<UINT32>(text.size()), textFormat.get(),
		bounds.right - bounds.left, bounds.bottom - bounds.top,
		textLayout.put()
	);
	winrt::com_ptr<IDWriteTextLayout3> textLayout3 = textLayout.as<IDWriteTextLayout3>();
	textLayout3->SetLineSpacing(&lineSpacingSettings);
	for (std::size_t i = 0; i != text.size(); ++i) {
		const wchar_t c = text[i];
		if (c >= L'0' && c <= L'9') {
			const float spacing = digitSpacings[c - L'0'];
			textLayout3->SetCharacterSpacing(spacing, spacing, 0, {static_cast<UINT32>(i), 1});
		}
	}
	return textLayout3;
}

void FixedWidthDigitTextRenderer::drawPreparedLayout(
	const winrt::com_ptr<IDWriteTextLayout3> &layout,
	const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush
) const {
	commonResources.renderTarget->DrawTextLayout(
		{bounds.left, bounds.top + (bounds.bottom - bounds.top - lineHeight) / 2 + verticalOffset},
		layout.get(), brush.get(), D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
	);
}

float FixedWidthDigitTextRenderer::getLineHeight() const {
	return lineHeight;
}

} // namespace CsgoHud