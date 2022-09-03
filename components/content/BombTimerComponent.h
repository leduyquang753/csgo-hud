#ifndef CSGO_HUD_COMPONENTS_CONTENT_BOMBTIMERCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_BOMBTIMERCOMPONENT_H

#include <optional>
#include <string>

#include "pch.h"

#include "components/base/Component.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"
#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that displays the time left to plant the bomb, until the bomb explodes or get defused.
*/
class BombTimerComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush>
			textBrush,
			gaugeOuterBrush, gaugeInnerPlantingBrush, gaugeInnerPlantedBrush,
			bombOpaqueBrush, bombTransparentBrush,
			defuseBlueBrush, defuseRedBrush;
		winrt::com_ptr<IDWriteTextFormat> textFormat;
		std::optional<FixedWidthDigitTextRenderer> textRenderer;
		winrt::com_ptr<ID2D1Layer> bombLayer, defuseLayer;

		TransitionedValue bombTransition, defuseTransition;
		// A "displayed bomb state" is needed for the fade out of the component.
		std::string bombState, displayedBombState;
		int bombTimeLeft = 0, defuseTimeLeft = 0, oldBombTime;

		void advanceTime(const int timePassed);
		void receiveData(const JSON &json);
	public:
		BombTimerComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_BOMBTIMERCOMPONENT_H