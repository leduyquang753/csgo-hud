#ifndef CSGO_HUD_TEXT_FIXEDWIDTHDIGITTEXTRENDERER_H
#define CSGO_HUD_TEXT_FIXEDWIDTHDIGITTEXTRENDERER_H

#include <array>
#include <string_view>

#include "pch.h"

namespace CsgoHud {

struct CommonResources;

/*
	A helper class for rendering text with digits having a uniform width to prevent number displays from wobbling
	when their values change.
*/
class FixedWidthDigitTextRenderer final {
	private:
		CommonResources &commonResources;
		const winrt::com_ptr<IDWriteTextFormat> textFormat;
		std::array<float, 10> digitSpacings = {};
	public:
		FixedWidthDigitTextRenderer(
			CommonResources &commonResources, const winrt::com_ptr<IDWriteTextFormat> &textFormat
		);
		void draw(std::wstring_view text, const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush) const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_TEXT_FIXEDWIDTHDIGITTEXTRENDERER_H