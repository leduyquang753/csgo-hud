#ifndef CSGO_HUD_COMPONENTS_BASE_SIZEDCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_SIZEDCOMPONENT_H

#include <memory>

#include "pch.h"

#include "components/base/SingleChildComponent.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component with specified size rather than taking all of the containing component's space.
*/
class SizedComponent final: public SingleChildComponent {
	protected:
		void paintChild(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
	public:
		// If a mode value is zero, the corresponding value is in pixels; otherwise, the value is a ratio.
		static constexpr UINT32 MODE_PIXELS = 0, MODE_RATIO = 1;
		
		// The size of the component. The ratio is relative to the size of the containing component.
		D2D1_SIZE_F size;
		D2D1_POINT_2U sizeMode;
		// The anchor point of the component. The ratio is relative to the size of the component.
		D2D1_POINT_2F anchor;
		D2D1_POINT_2U anchorMode;
		/*
			The offset of the component. The component will be shifted so that the anchor point is positioned at the
			specified offset point. The ratio is relative to the size of the containing component.
		*/
		D2D1_POINT_2F offset;
		D2D1_POINT_2U offsetMode;

		SizedComponent(
			CommonResources &commonResources,
			const D2D1_SIZE_F &size, const D2D1_POINT_2U &sizeMode,
			const D2D1_POINT_2F &anchor, const D2D1_POINT_2U &anchorMode,
			const D2D1_POINT_2F &offset, const D2D1_POINT_2U &offsetMode,
			std::unique_ptr<Component> &&child = nullptr
		);
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_SIZEDCOMPONENT_H