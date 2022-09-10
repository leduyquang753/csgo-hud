#ifndef CSGO_HUD_COMPONENTS_CONTENT_ALLPLAYERSCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_ALLPLAYERSCOMPONENT_H

#include <array>
#include <memory>
#include <optional>

#include "pch.h"

#include "components/base/BagComponent.h"
#include "components/base/Component.h"
#include "components/content/PlayerInfoComponent.h"
#include "movement/TransitionedValue.h"
#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

struct CommonResources;
class PlayerInfoComponent;
class StatsHeaderComponent;

/*
	A component that displays every player's information to either side of the HUD.
*/
class AllPlayersComponent final: public Component {
	private:
		std::optional<PlayerInfoComponent::Resources> resources;
		std::optional<FixedWidthDigitTextRenderer> normalTextRenderer, boldTextRenderer;

		std::unique_ptr<BagComponent> container;
		std::array<PlayerInfoComponent*, 10> children;
		StatsHeaderComponent *leftStatsHeader, *rightStatsHeader;

		std::string phase;
		bool statsOn = false;
		TransitionedValue statsTransition;
		
		int activeSlot = -1;

		void receivePhaseData(JSON::dom::object &json);
		void receivePlayerData(JSON::dom::object &json);
	public:
		AllPlayersComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_ALLPLAYERSCOMPONENT_H