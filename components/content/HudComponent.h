#ifndef CSGO_HUD_COMPONENTS_CONTENT_HUDCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_HUDCOMPONENT_H

#include <memory>

#include "components/base/Component.h"
#include "components/base/BagComponent.h"

namespace CsgoHud {

struct CommonResources;

/*
	The main component that contains all elements of the HUD.
*/
class HudComponent final: public Component {
	private:
		std::unique_ptr<BagComponent> bag;
		bool shown = true;
		
		void onVisibilityToggle();
	public:
		HudComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_HUDCOMPONENT_H