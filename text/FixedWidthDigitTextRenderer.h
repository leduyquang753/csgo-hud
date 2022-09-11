#ifndef CSGO_HUD_TEXT_FIXEDWIDTHDIGITTEXTRENDERER_H
#define CSGO_HUD_TEXT_FIXEDWIDTHDIGITTEXTRENDERER_H

#include <array>
#include <string_view>

#include "pch.h"

#include "text/TextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A helper class for rendering text with digits having a uniform width to prevent number displays from wobbling
	when their values change.
*/
class FixedWidthDigitTextRenderer final: public TextRenderer {
	private:
		const winrt::com_ptr<IDWriteTextFormat> textFormat;
		const DWRITE_LINE_SPACING lineSpacingSettings;
		std::array<float, 10> digitSpacings = {};
		const float verticalOffset;
		const float lineHeight;
	public:
		FixedWidthDigitTextRenderer(
			CommonResources &commonResources, const winrt::com_ptr<IDWriteTextFormat> &textFormat,
			float offsetRatio, // The proportion to the font size that the text should be vertically shifted.
			float lineHeightRatio // The line height propertion to the font size.
		);
		winrt::com_ptr<IDWriteTextLayout3> prepareLayout(
			std::wstring_view text, const D2D1_RECT_F &bounds
		) const override;
		void drawPreparedLayout(
			const winrt::com_ptr<IDWriteTextLayout3> &layout,
			const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush
		) const override;
		float getLineHeight() const override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_TEXT_FIXEDWIDTHDIGITTEXTRENDERER_H