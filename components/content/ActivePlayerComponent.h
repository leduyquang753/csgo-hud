#ifndef CSGO_HUD_COMPONENTS_CONTENT_ACTIVEPLAYERCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_ACTIVEPLAYERCOMPONENT_H

#include <optional>
#include <string>

#include "pch.h"

#include "components/base/Component.h"
#include "data/PlayerData.h"
#include "movement/TransitionedValue.h"
#include "text/NormalTextRenderer.h"
#include "text/FixedWidthDigitTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	A component that displays additional information about the currently active player.
*/
class ActivePlayerComponent final: public Component {
	private:
		winrt::com_ptr<ID2D1Layer> layer;
		winrt::com_ptr<ID2D1SolidColorBrush>
			backgroundBlackBrush, backgroundCtBrush, backgroundTBrush, ctBrush, tBrush, whiteBrush;
		winrt::com_ptr<IDWriteTextFormat> bigNumberTextFormat;
		std::optional<NormalTextRenderer> nameTextRenderer, statsTextRenderer, weaponTextRenderer;
		std::optional<FixedWidthDigitTextRenderer> bigNumberRenderer, smallNumberRenderer;
		const TransitionedValue &masterTransition;
		TransitionedValue selfTransition;

		PlayerData player;
		bool shown = false;
	public:
		ActivePlayerComponent(CommonResources &commonResources, const TransitionedValue &masterTransition);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_ACTIVEPLAYERCOMPONENT_H