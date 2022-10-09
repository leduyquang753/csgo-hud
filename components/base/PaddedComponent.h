#ifndef CSGO_HUD_COMPONENTS_BASE_PADDEDCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_PADDEDCOMPONENT_H

#include <memory>

#include "components/base/SingleChildComponent.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component with padding to the four sides to shrink the size of a child component.
*/
class PaddedComponent final: public SingleChildComponent {
	protected:
		void paintChild(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
	public:
		D2D1_RECT_F padding;
		PaddedComponent(CommonResources &commonResources, std::unique_ptr<Component> &&child = nullptr);
		PaddedComponent(
			CommonResources &commonResources, float padding, std::unique_ptr<Component> &&child = nullptr
		);
		PaddedComponent(
			CommonResources &commonResources, float paddingHorizontal, float paddingVertical,
			std::unique_ptr<Component> &&child = nullptr
		);
		PaddedComponent(
			CommonResources &commonResources,
			float paddingLeft, float paddingTop, float paddingRight, float paddingBottom,
			std::unique_ptr<Component> &&child = nullptr
		);
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_PADDEDCOMPONENT_H