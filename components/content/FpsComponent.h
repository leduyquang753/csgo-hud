#ifndef CSGO_HUD_COMPONENTS_CONTENT_FPSCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_FPSCOMPONENT_H

#include <optional>

#include "pch.h"

#include "components/base/Component.h"
#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	The component that displays the frames per second count for performance diagnostic.
*/
class FpsComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush> textBrush;
		std::optional<FixedWidthDigitTextRenderer> textRenderer;

		int currentSecond = 0;
		int lastFps = 0, fps = 0;

		bool shown = false;

		void onVisibilityToggle();
	public:
		FpsComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_FPSCOMPONENT_H