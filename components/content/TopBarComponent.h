#ifndef CSGO_HUD_COMPONENTS_CONTENT_TOPBARCOMPONENT_H
#define CSGO_HUD_COMPONENTS_CONTENT_TOPBARCOMPONENT_H

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "pch.h"

#include "components/base/Component.h"
#include "components/base/StackComponent.h"
#include "text/FixedWidthDigitTextRenderer.h"
#include "text/NormalTextRenderer.h"

namespace CsgoHud {

struct CommonResources;

/*
	The portion to the top of the HUD, including scores, clock and bomb timer.
*/
class TopBarComponent final: public Component {
	private:
		class ChildComponent final: public Component {
			public:
				winrt::com_ptr<ID2D1SolidColorBrush> backgroundBrush, textBrush;
				const TextRenderer &textRenderer;
				std::wstring text;

				ChildComponent(
					CommonResources &commonResources,
					const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
					const TextRenderer &textRenderer,
					std::wstring_view text
				);
				void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
		};

		winrt::com_ptr<ID2D1SolidColorBrush>
			ctNameBackgroundBrush, tNameBackgroundBrush, ctScoreBackgroundBrush, tScoreBackgroundBrush,
			textBrush;
		std::optional<NormalTextRenderer> nameTextRenderer;
		std::optional<FixedWidthDigitTextRenderer> scoreTextRenderer;
		
		std::unique_ptr<StackComponent> container;
		ChildComponent
			*leftNameDisplay, *rightNameDisplay, *leftScoreDisplay, *rightScoreDisplay,
			*ctNameDisplay, *tNameDisplay, *ctScoreDisplay, *tScoreDisplay;
		float *leftNameWidth, *rightNameWidth;
		bool ctToTheLeft = true;

		void receivePlayerData(JSON::dom::object &json);
		void receiveMapData(JSON::dom::object &json);
		void updateCtSide(bool toTheLeft);
	public:
		TopBarComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_TOPBARCOMPONENT_H