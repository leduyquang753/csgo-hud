#ifndef CSGO_HUD_DATA_CONFIGURATIONDATA_H
#define CSGO_HUD_DATA_CONFIGURATIONDATA_H

#include <string>

namespace CsgoHud {

/*
	Contains the HUD's configuration data retrieved from `Configuration.json`.
*/
class ConfigurationData final {
	private:
		JSON::dom::parser jsonParser;
	public:
		int httpServerPort;
		int windowWidth, windowHeight;
		float hudWidth, hudHeight;
		float hudScaling;
		bool startingPositionCenter;
		int windowOffsetX, windowOffsetY;
		std::wstring fontFamily;
		float fontOffsetRatio;
		float fontLineHeightRatio;
		std::wstring defaultCtName, defaultTName;
		struct {
			int freezeTime;
			int mainTime;
			int bombTime;
			int roundEnd;
			int halfTime;
			int timeout;
		} timings;
		struct {
			bool addCurrentClipToTotalAmmo;
			bool decimalSeparatorComma;
			bool showSecondsSign;
			bool dollarSignAfter;
			bool dollarSignWithSpace;
			bool showTenthPlayerAsZero;
		} formatting;
		struct {
			D2D1_COLOR_F ctPrimary;
			D2D1_COLOR_F ctSecondary;
			D2D1_COLOR_F tPrimary;
			D2D1_COLOR_F tSecondary;
			D2D1_COLOR_F damage;
			D2D1_COLOR_F notEnoughTimeToDefuse;
		} colors;
		JSON::dom::object keybindings;

		ConfigurationData();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_CONFIGURATIONDATA_H