#ifndef CSGO_HUD_COMPONENTS_CONTENT_PLAYERINFOCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_PLAYERINFOCOMPONENT_H

#include <cstdint>

#include "components/base/Component.h"
#include "movement/TransitionedValue.h"

namespace CsgoHud {

struct CommonResources;
class FixedWidthDigitTextRenderer;
class TransitionedValue;

/*
	A component that displays a player's information to either side of the HUD.
*/
class PlayerInfoComponent final: public Component {
	public:
		struct Resources {
			D2D1_COLOR_F
				backgroundInactiveColor, backgroundActiveColor, activeOutlineColor,
				weaponInactiveColor, weaponEmptyActiveColor, weaponEmptyInactiveColor,
				flashColor, smokeColor, fireColor;
			winrt::com_ptr<ID2D1SolidColorBrush>
				teamCtBrush, teamTBrush, healthBrush, textWhiteBrush, textGreenBrush;
			winrt::com_ptr<IDWriteTextFormat> normalTextFormat, boldTextFormat;
			FixedWidthDigitTextRenderer &normalTextRenderer, &boldTextRenderer;
			TransitionedValue &statsTransition;
		};
	private:
		const Resources &resources;
		winrt::com_ptr<ID2D1Layer> statsLayer;

		const bool rightSide;
		TransitionedValue activeTransition, healthTransition;
		int currentHealth, oldHealth = 0;
		int healthDecayTime = 0;
		int lastIndex = -1;
		std::uint64_t lastPlayerId;
		bool wasActive = false;

		void advanceTime(int timePassed);
	public:
		bool active = false;
		int index = -1;

		PlayerInfoComponent(CommonResources &commonResources, bool rightSide, const Resources &resources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_PLAYERINFOCOMPONENT_H