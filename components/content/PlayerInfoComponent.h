#ifndef CSGO_HUD_COMPONENTS_CONTENT_PLAYERINFOCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_PLAYERINFOCOMPONENT_H

#include "pch.h"

#include "components/base/Component.h"
#include "movement/TransitionedValue.h"

namespace CsgoHud {

struct CommonResources;
class FixedWidthDigitTextRenderer;

/*
	A component that displays a player's information to either side of the HUD.
*/
class PlayerInfoComponent final: public Component {
	private:
		const D2D1_COLOR_F backgroundInactiveColor, backgroundActiveColor, activeOutlineColor;
		const winrt::com_ptr<ID2D1SolidColorBrush>
			teamCtBrush, teamTBrush, healthBrush, textWhiteBrush, textGreenBrush;
		winrt::com_ptr<ID2D1Effect> inactiveEffect, emptyActiveEffect, emptyInactiveEffect;
		winrt::com_ptr<IDWriteTextFormat> normalTextFormat, boldTextFormat;
		FixedWidthDigitTextRenderer &normalTextRenderer, &boldTextRenderer;
		
		TransitionedValue activeTransition, healthTransition;
		int currentHealth, oldHealth = 0;
		int healthDecayTime = 0;
		int lastIndex = -1;
		bool wasActive = false;

		void advanceTime(int timePassed);
	public:
		bool active = false;
		int index = -1;

		PlayerInfoComponent(
			CommonResources &commonResources,
			const D2D1_COLOR_F &backgroundInactiveColor,
			const D2D1_COLOR_F &backgroundActiveColor,
			const D2D1_COLOR_F &activeOutlineColor,
			const winrt::com_ptr<ID2D1SolidColorBrush> &teamCtBrush,
			const winrt::com_ptr<ID2D1SolidColorBrush> &teamTBrush,
			const winrt::com_ptr<ID2D1SolidColorBrush> &healthBrush,
			const winrt::com_ptr<ID2D1SolidColorBrush> &textWhiteBrush,
			const winrt::com_ptr<ID2D1SolidColorBrush> &textGreenBrush,
			const winrt::com_ptr<IDWriteTextFormat> &normalTextFormat,
			const winrt::com_ptr<IDWriteTextFormat> &boldTextFormat,
			FixedWidthDigitTextRenderer &normalTextRenderer,
			FixedWidthDigitTextRenderer &boldTextRenderer
		);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_PLAYERINFOCOMPONENT_H