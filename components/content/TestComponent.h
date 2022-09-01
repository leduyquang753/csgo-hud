#ifndef CSGO_HUD_COMPONENTS_CONTENT_TESTCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_TESTCOMPONENT_H

#include "pch.h"

#include "components/base/Component.h"
#include "movement/LinearMovementFunction.h"
#include "movement/CubicBezierMovementFunction.h"

namespace CsgoHud {

struct CommonResources;

class TestComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush> brush;
		winrt::com_ptr<IDWriteTextFormat> textFormat;
		int count = 0;
		LinearMovementFunction linear;
		CubicBezierMovementFunction cubicBezier;
	public:
		TestComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_TESTCOMPONENT_H