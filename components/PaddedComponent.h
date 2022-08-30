#ifndef CSGO_HUD_COMPONENTS_PADDEDCOMPONENT_H
#define CSGO_HUD_COMPONENTS_PADDEDCOMPONENT_H

#include <memory>

#include "pch.h"

#include "components/Component.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component with padding to the four sides to shrink the size of a child component.
*/
class PaddedComponent final: public Component {
	public:
		D2D1_RECT_F padding;
		std::unique_ptr<Component> child;
		PaddedComponent(CommonResources &commonResources);
		PaddedComponent(CommonResources &commonResources, float padding);
		PaddedComponent(CommonResources &commonResources, float paddingHorizontal, float paddingVertical);
		PaddedComponent(
			CommonResources &commonResources,
			float paddingLeft, float paddingTop, float paddingRight, float paddingBottom
		);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_PADDEDCOMPONENT_H