#ifndef CSGO_HUD_COMPONENTS_CONTENT_TEAMBUYCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_TEAMBUYCOMPONENT_H

#include <optional>

#include "pch.h"

#include "components/base/Component.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "text/NormalTextRenderer.h"

namespace CsgoHud {

struct CommonResources;
class TransitionedValue;

/*
	The component that displays each team's total buy amount and equipment value as a power comparison.
*/
class TeamBuyComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1Layer> layer;
		winrt::com_ptr<ID2D1SolidColorBrush> backgroundBrush, ctBrush, tBrush, textBrush;
		winrt::com_ptr<IDWriteTextFormat> amountTextFormat;
		std::optional<NormalTextRenderer> titleTextRenderer;
		std::optional<FixedWidthDigitTextRenderer> amountTextRenderer;
		const TransitionedValue &masterTransition;
	public:
		TeamBuyComponent(CommonResources &commonResources, const TransitionedValue &masterTransition);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_TEAMBUYCOMPONENT_H