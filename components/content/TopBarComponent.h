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
class TransitionedValue;

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
		class WinLoseComponent final: public Component {
			public:
				winrt::com_ptr<ID2D1Layer> layer;
				winrt::com_ptr<ID2D1SolidColorBrush>
					backgroundBlackBrush, backgroundTeamBrush, teamBrush, textBrush, moneyGainBrush;
				const TextRenderer &winTextRenderer, &streakTextRenderer, &moneyGainTextRenderer;
				const TransitionedValue &transition;
				// -1 if it's a loss, otherwise it's a win.
				int winIconIndex;
				std::wstring streak;
				int lossBonusLevel;
				std::wstring moneyGain;

				WinLoseComponent(
					CommonResources &commonResorces,
					const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBlackBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundTeamBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &teamBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &moneyGainBrush,
					const TextRenderer &winTextRenderer,
					const TextRenderer &streakTextRenderer,
					const TextRenderer &moneyGainTextRenderer,
					const TransitionedValue &transition
				);
				void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
		};
		class MatchPointComponent final: public Component {
			public:
				winrt::com_ptr<ID2D1Layer> layer;
				winrt::com_ptr<ID2D1SolidColorBrush> backgroundBrush, textBrush;
				const TextRenderer &textRenderer;
				const TransitionedValue &transition;
				// 0 if it's not this team's match point.
				int matchPoints;

				MatchPointComponent(
					CommonResources &commonResources,
					const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
					const TextRenderer &textRenderer,
					const TransitionedValue &transition
				);
				void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
		};
		class TimeoutComponent final: public Component {
			public:
				winrt::com_ptr<ID2D1Layer> layer;
				winrt::com_ptr<ID2D1SolidColorBrush> backgroundBrush, textBrush;
				const TextRenderer &textRenderer;
				const TransitionedValue &transition;
				// -1 if it's not this team's timeout.
				int timeoutsRemaining;

				TimeoutComponent(
					CommonResources &commonResources,
					const winrt::com_ptr<ID2D1SolidColorBrush> &backgroundBrush,
					const winrt::com_ptr<ID2D1SolidColorBrush> &textBrush,
					const TextRenderer &textRenderer,
					const TransitionedValue &transition
				);
				void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
		};

		winrt::com_ptr<ID2D1SolidColorBrush>
			backgroundBlackBrush,
			ctNameBackgroundBrush, tNameBackgroundBrush, ctScoreBackgroundBrush, tScoreBackgroundBrush,
			textBrush, moneyGainBrush;
		std::optional<NormalTextRenderer> nameTextRenderer, winLoseTextRenderer;
		std::optional<FixedWidthDigitTextRenderer> scoreTextRenderer, streakTextRenderer, moneyGainTextRenderer;
		
		std::unique_ptr<StackComponent> container;
		ChildComponent
			*leftNameDisplay, *rightNameDisplay, *leftScoreDisplay, *rightScoreDisplay,
			*ctNameDisplay, *tNameDisplay, *ctScoreDisplay, *tScoreDisplay;
		float *leftNameWidth, *rightNameWidth, *paddingHeight;
		WinLoseComponent *leftWinLoseDisplay, *rightWinLoseDisplay;
		MatchPointComponent *leftMatchPointDisplay, *rightMatchPointDisplay;
		TimeoutComponent *leftTimeoutDisplay, *rightTimeoutDisplay;
		TransitionedValue winLoseTransition, matchPointTransition, timeoutTransition;
		bool winLoseShown = false, matchPointShown = false, timeoutShown = false;

		int ctScore = 0, tScore = 0, ctTimeouts = 0, tTimeouts = 0;
		
		bool ctToTheLeft = true;

		void receiveMapData(JSON::dom::object &json);
		void updateCtSide(bool toTheLeft);
	public:
		TopBarComponent(CommonResources &commonResources);
		void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) override;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_CONTENT_TOPBARCOMPONENT_H