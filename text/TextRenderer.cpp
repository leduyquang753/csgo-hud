#include "pch.h"

#include "text/TextRenderer.h"

namespace CsgoHud {

struct CommonResources;

// == TextRenderer ==

TextRenderer::TextRenderer(CommonResources &commonResources): commonResources(commonResources) {}

void TextRenderer::draw(
	std::wstring_view text, const D2D1_RECT_F &bounds, const winrt::com_ptr<ID2D1Brush> &brush
) const {
	drawPreparedLayout(prepareLayout(text, bounds), bounds, brush);
}

} // namespace CsgoHud