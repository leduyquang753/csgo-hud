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
		int windowWidth, windowHeight;
		float hudWidth, hudHeight;
		float hudScaling;
		std::wstring fontFamily;
		JSON::dom::object keybindings;

		ConfigurationData();
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_CONFIGURATIONDATA_H