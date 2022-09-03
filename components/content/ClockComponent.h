#ifndef CSGO_HUD_COMPONENTS_CONTENT_CLOCKCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_CLOCKCOMPONENT_H

#include <optional>
#include <string>

#include "pch.h"

#include "components/base/Component.h"
#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that displays the time left of the current phase of the game.
*/
class ClockComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush>
			textWhiteBrush, textRedBrush,
			backgroundBrush, progressWhiteBrush, progressRedBrush;
		winrt::com_ptr<IDWriteTextFormat> textFormat;
		std::optional<FixedWidthDigitTextRenderer> textRenderer;

		std::string phase;
		int phaseTimeLeft;

		void advanceTime(const int timePassed);
		void receiveData(const JSON &json);
		int getPhaseTime();
	public:
		ClockComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_CLOCKCOMPONENT_H