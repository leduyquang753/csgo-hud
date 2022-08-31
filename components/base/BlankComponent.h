#ifndef CSGO_HUD_COMPONENTS_BASE_BLANKCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_BLANKCOMPONENT_H

#include "pch.h"

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that paints nothing.
*/
class BlankComponent final: public Component {
	public:
		BlankComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_BLANKCOMPONENT_H