#ifndef CSGO_HUD_COMPONENTS_BAGCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BAGCOMPONENT_H

#include "pch.h"

#include <memory>
#include <vector>

#include "components/Component.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that is just a simple container of other components.
*/
class BagComponent final: public Component {
	public:
		std::vector<std::unique_ptr<Component>> children;

		BagComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BAGCOMPONENT_H