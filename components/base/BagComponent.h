#ifndef CSGO_HUD_COMPONENTS_BASE_BAGCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_BAGCOMPONENT_H

#include <memory>
#include <vector>

#include "components/base/Component.h"
#include "components/base/MultipleChildrenComponent.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that is just a simple container of other components.
*/
class BagComponent final: public MultipleChildrenComponent {
	public:
		BagComponent(CommonResources &commonResources, std::vector<std::unique_ptr<Component>> &&children = {});
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_BAGCOMPONENT_H