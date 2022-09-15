#ifndef CSGO_HUD_COMPONENTS_CONTENT_MINIMAP_COMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_MINIMAP_COMPONENT_H

#include <optional>

#include "pch.h"

#include "components/base/Component.h"
#include "text/NormalTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that displays the overview of the map and positions of the players and items on it.
*/
class MinimapComponent final: public Component {
	private:
		D2D1_COLOR_F
			droppedBombColor, plantedBombColor, defusedBombColor,
			ctColor, tColor;

		winrt::com_ptr<ID2D1Layer> layer1, layer2;
		winrt::com_ptr<ID2D1SolidColorBrush>
			whiteBrush, ctBrush, tBrush, bombsiteNameBrush, bombBrush, flashBrush, smokeBrush, fireBrush;
		std::optional<NormalTextRenderer> numberRenderer, bombsiteNameRenderer;
	public:
		MinimapComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_MINIMAP_COMPONENT_H