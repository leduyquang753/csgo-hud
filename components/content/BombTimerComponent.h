#ifndef CSGO_HUD_COMPONENTS_CONTENT_BOMBTIMERCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_BOMBTIMERCOMPONENT_H

#include <optional>
#include <string>

#include "components/base/Component.h"
#include "data/BombData.h"
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
		BombData::State bombState = BombData::State::DROPPED, displayedBombState;
		int oldBombTime;
	public:
		BombTimerComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_BOMBTIMERCOMPONENT_H