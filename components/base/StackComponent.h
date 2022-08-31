#ifndef CSGO_HUD_COMPONENTS_BASE_STACKCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_STACKCOMPONENT_H

#include <vector>

#include "pch.h"

#include "components/base/Component.h"
#include "components/base/StackComponentChild.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that arranges its children along either the horizontal or vertical axis.
*/
class StackComponent final: public Component {
	public:
		// If the axis member is `false`, it is horizontal, if it's `true`, it's vertical.
		static const bool AXIS_HORIZONTAL = false, AXIS_VERTICAL = true;
		/*
			If the axis direction member is `false`, the axis goes right or down, if it's true, the axis goes left
			or up.
		*/
		static const bool AXIS_INCREASE = false, AXIS_DECRASE = true;
		/*
			If the axis position mode is `false`, the axis position is in pixels, if it's true, the axis position is
			a ratio of the dimension perpendicular to the axis.
		*/
		static const bool MODE_PIXELS = false, MODE_RATIO = true;

		bool axis;
		bool axisDirection;
		float axisPosition;
		bool axisPositionMode;
		
		std::vector<StackComponentChild> children;
		
		StackComponent(
			CommonResources &commonResources,
			bool axisDirection, float axisPosition, bool axisPositionMode,
			std::vector<StackComponentChild> &&children = {}
		);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_STACKCOMPONENT_H