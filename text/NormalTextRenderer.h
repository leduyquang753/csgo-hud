#ifndef CSGO_HUD_NORMALTEXTRENDERER_H
#define CSGO_HUD_NORMALTEXTRENDERER_H

#include <string_view>

#include "pch.h"

#include "text/TextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A text rendering helper because DirectWrite tends to be yanky about baseline positions.
*/
class NormalTextRenderer final: public TextRenderer {
	private:
		const winrt::com_ptr<IDWriteTextFormat> textFormat;
		const DWRITE_LINE_SPACING lineSpacingSettings;
		const float verticalOffset;
		float lineHeight;
	public:
		NormalTextRenderer(
			CommonResources &commonResources, const winrt::com_ptr<IDWriteTextFormat> &textFormat,
			float offsetRatio, // The proportion to the font size that the text should be vertically shifted.
			float lineHeightRatio // The line height propertion to the font size.
		);
		void draw(
			std::wstring_view text, const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush
		) const override;
		float getLineHeight() const override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_NORMALTEXTRENDERER_H