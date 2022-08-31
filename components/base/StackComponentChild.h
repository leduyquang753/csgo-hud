#ifndef CSGO_HUD_COMPONENTS_BASE_STACKCOMPONENTCHILD_H
#define CSGO_HUD_COMPONENTS_BASE_STACKCOMPONENTCHILD_H

#include <memory>

#include "pch.h"

namespace CsgoHud {

/*
	A child of `StackComponent`, storing size and alignment data.
*/
struct StackComponentChild final {
	public:
		// If a mode value is zero, the corresponding value is in pixels; otherwise, the value is a ratio.
		static const UINT32	MODE_PIXELS = 0, MODE_RATIO = 1;
		
		// The size of the child. The ratio is relative to the size of the stack component.
		D2D1_SIZE_F size;
		D2D1_POINT_2U sizeMode;
		// The anchor position of the child. The ratio is relative to the size of the component.
		float anchor;
		bool anchorMode;
		/*
			The offset amount of the child. The component will be shifted perpendicular to the stack component's
			axis so that the anchor axis is at the specified amount of offset compared to the stack component's
			main axis. The ratio is relative to the size of the stack component.
		*/
		float offset;
		bool offsetMode;

		std::unique_ptr<Component> component;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_STACKCOMPONENTCHILD_H