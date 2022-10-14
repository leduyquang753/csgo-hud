#ifndef CSGO_HUD_COMPONENTS_CONTENT_UTILITYCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_UTILITYCOMPONENT_H

#include <optional>

#include "components/base/Component.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "text/NormalTextRenderer.h"

namespace CsgoHud {

struct CommonResources;
class TransitionedValue;

/*
	A component that displays each team's total number of grenades overall as well as of each type.
*/
class UtilityComponent final: public Component {
	private:
		D2D1_COLOR_F normalColor, grayedColor;
		winrt::com_ptr<ID2D1SolidColorBrush>
			ctBrush, tBrush, headerShadeBrush, backgroundBrush, backgroundShadeBrush,
			normalTextBrush, grayedTextBrush;
		std::optional<FixedWidthDigitTextRenderer> titleTextRenderer, totalTextRenderer, countTextRenderer;
		winrt::com_ptr<ID2D1Layer> layer;
		const TransitionedValue &transition;

		bool rightSide;
	public:
		bool ct;
		
		UtilityComponent(CommonResources &commonResources, bool rightSide, const TransitionedValue &transition);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_UTILITYCOMPONENT_H