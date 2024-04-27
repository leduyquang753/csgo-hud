#ifndef CSGO_HUD_COMPONENTS_CONTENT_BOTTOMBARCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_BOTTOMBARCOMPONENT_H

#include <memory>

#include "components/base/BagComponent.h"
#include "components/base/Component.h"
#include "movement/TransitionedValue.h"

namespace CsgoHud {

struct CommonResources;

/*
	The portion to the bottom of the HUD, including the active player info, team buy comparison and round history.
*/
class BottomBarComponent final: public Component {
	private:
		std::unique_ptr<BagComponent> container;
		TransitionedValue fadingTransition, slidingTransition1, slidingTransition2;

		bool activePlayerShown = false, forceShowTeamBuy = false, showRoundHistory = false;

		void onForceShowTeamBuyToggle();
		void onShowRoundHistoryToggle();
	public:
		BottomBarComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_BOTTOMBARCOMPONENT_H