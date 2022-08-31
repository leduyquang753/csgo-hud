#ifndef CSGO_HUD_COMPONENTS_BASE_MULTIPLECHILDRENCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_MULTIPLECHILDRENCOMPONENT_H

#include <memory>
#include <vector>

#include "pch.h"

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component with an arbitrary number of children.
*/
class MultipleChildrenComponent: public Component {
	public:
		std::vector<std::unique_ptr<Component>> children;

		MultipleChildrenComponent(
			CommonResources &commonResources, std::vector<std::unique_ptr<Component>> &&children = {}
		);
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_MULTIPLECHILDRENCOMPONENT_H