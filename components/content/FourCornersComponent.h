#ifndef CSGO_HUD_COMPONENTS_CONTENT_FOURCORNERSCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_FOURCORNERSCOMPONENT_H

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that displays four chevrons to the corner to help align the HUD window with the game window.
*/
class FourCornersComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1SolidColorBrush> brush;

		bool shown = true;
		
		void onVisibilityToggle();
	public:
		FourCornersComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_FOURCORNERSCOMPONENT_H