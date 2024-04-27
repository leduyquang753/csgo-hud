#ifndef CSGO_HUD_COMPONENTS_BASE_SINGLECHILDCOMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_SINGLECHILDCOMPONENT_H

#include <memory>

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component with a single child.
*/
class SingleChildComponent: public Component {
	protected:
		virtual void paintChild(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) = 0;
	public:
		std::unique_ptr<Component> child;

		SingleChildComponent(CommonResources &commonResources, std::unique_ptr<Component> &&child = nullptr);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) final override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_SINGLECHILDCOMPONENT_H