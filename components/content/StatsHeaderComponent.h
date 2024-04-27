#ifndef CSGO_HUD_COMPONENTS_CONTENT_STATSHEADERCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_STATSHEADERCOMPONENT_H

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;
struct PlayerInfoComponent::Resources;

/*
	The component that sits on top of each side displaying the headers for the statistics table.
*/
class StatsHeaderComponent final: public Component {
	private:
		const PlayerInfoComponent::Resources &resources;
		winrt::com_ptr<ID2D1Layer> layer;

		const bool rightSide;
	public:
		bool ct = false;

		StatsHeaderComponent(
			CommonResources &commonResources, bool rightSide, const PlayerInfoComponent::Resources &resources
		);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_STATSHEADERCOMPONENT_H