#ifndef CSGO_HUD_COMPONENTS_CONTENT_ROUNDHISTORYCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_ROUNDHISTORYCOMPONENT_H

#include <optional>

#include "components/base/Component.h"
#include "text/NormalTextRenderer.h"

namespace CsgoHud {

struct CommonResources;
class TransitionedValue;

/*
	A component that displays the results of the previous rounds.
*/
class RoundHistoryComponent final: public Component {
	private:
		D2D1_COLOR_F ctColor, tColor;
		winrt::com_ptr<ID2D1SolidColorBrush>
			backgroundBrush, inactiveRoundBrush, activeRoundBrush, highlightBrush, ctBrush, tBrush, textBrush;
		std::optional<NormalTextRenderer> titleTextRenderer, numberTextRenderer;
		winrt::com_ptr<ID2D1Layer> layer;
		const TransitionedValue &transition;
	public:
		RoundHistoryComponent(CommonResources &commonResources, const TransitionedValue &transition);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_ROUNDHISTORYCOMPONENT_H