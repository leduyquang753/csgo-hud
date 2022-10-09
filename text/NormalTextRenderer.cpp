#include "pch.h"

#include <string_view>

#include "resources/CommonResources.h"
#include "text/TextRenderer.h"

#include "text/NormalTextRenderer.h"

namespace CsgoHud {

// == NormalTextRenderer ==

NormalTextRenderer::NormalTextRenderer(
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
{}

winrt::com_ptr<IDWriteTextLayout3> NormalTextRenderer::prepareLayout(
	std::wstring_view text, const D2D1_RECT_F &bounds
) const {
	winrt::com_ptr<IDWriteTextLayout> textLayout;
	winrt::com_ptr<IDWriteTextLayout3> textLayout3;
	commonResources.writeFactory->CreateTextLayout(
		text.data(), static_cast<UINT32>(text.size()), textFormat.get(),
		bounds.right - bounds.left, bounds.bottom - bounds.top,
		textLayout.put()
	);
	textLayout->QueryInterface(textLayout3.put());
	textLayout3->SetLineSpacing(&lineSpacingSettings);
	return textLayout3;
}


void NormalTextRenderer::drawPreparedLayout(
	const winrt::com_ptr<IDWriteTextLayout3> &layout,
	const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush
) const {
	commonResources.renderTarget->DrawTextLayout(
		{bounds.left, bounds.top + (bounds.bottom - bounds.top - lineHeight) / 2 + verticalOffset},
		layout.get(), brush.get(), D2D1_DRAW_TEXT_OPTIONS_NO_SNAP
	);
}

float NormalTextRenderer::getLineHeight() const {
	return lineHeight;
}

} // namespace CsgoHud