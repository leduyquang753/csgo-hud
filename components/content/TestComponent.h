#ifndef CSGO_HUD_COMPONENTS_CONTENT_TESTCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_TESTCOMPONENT_H

#include <string>

#include "pch.h"

#include "components/base/Component.h"
#include "movement/CubicBezierMovementFunction.h"
#include "movement/TransitionedValue.h"

namespace CsgoHud {

struct CommonResources;

class TestComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush>
			textWhiteBrush, textRedBrush,
			timeBlackBrush, timeWhiteBrush, timeRedBrush,
			gaugeOuterBrush, gaugeInnerPlantingBrush, gaugeInnerPlantedBrush,
			bombOpaqueBrush, bombTransparentBrush,
			defuseBlueBrush, defuseRedBrush;
		winrt::com_ptr<IDWriteTextFormat> timeTextFormat, bombTextFormat;
		winrt::com_ptr<ID2D1Layer> bombLayer, defuseLayer;

		TransitionedValue bombPosition, bombOpacity, defusePosition, defuseOpacity;

		std::string bombState{}, oldBombState{}, phase{};
		int bombTimeLeft = 0, defuseTimeLeft = 0, phaseTimeLeft = 0, oldBombTime;

		void advanceTime(const int timePassed);
		void receiveBombData(const JSON &json);
		void receivePhaseData(const JSON &json);
		int getPhaseTime();
	public:
		TestComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_TESTCOMPONENT_H