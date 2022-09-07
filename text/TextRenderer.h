#ifndef CSGO_HUD_TEXT_TEXTRENDERER_H
#define CSGO_HUD_TEXT_TEXTRENDERER_H

#include <string_view>

#include "pch.h"

namespace CsgoHud {

struct CommonResources;

/*
	The base text renderer class.
*/
class TextRenderer {
	protected:
		CommonResources &commonResources;
	public:
		TextRenderer(CommonResources &commonResources);
		virtual ~TextRenderer() = default;
		// The text will be centered vertically in the bounds, the horizontal alignment is set in the text format.
		virtual void draw(
			std::wstring_view text, const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush
		) const = 0;
		virtual float getLineHeight() const = 0;
};

} // namespace CsgoHud

#endif // CSGO_HUD_TEXT_TEXTRENDERER_H