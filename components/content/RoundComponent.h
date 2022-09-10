#ifndef CSGO_HUD_COMPONENTS_CONTENT_ROUNDCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_ROUNDCOMPONENT_H

#include <optional>

#include "pch.h"

#include "components/base/Component.h"
#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that displays the current round number being played.
*/
class RoundComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush> textBrush;
		winrt::com_ptr<IDWriteTextFormat> textFormat;
		std::optional<FixedWidthDigitTextRenderer> textRenderer;
	public:
		RoundComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_ROUNDCOMPONENT_H