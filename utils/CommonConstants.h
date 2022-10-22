#ifndef CSGO_HUD_UTILS_COMMONCONSTANTS_H
#define CSGO_HUD_UTILS_COMMONCONSTANTS_H

namespace CsgoHud {

namespace CommonConstants {
	// The custom window message sent by the HTTP server to notify the HUD winodw about new data being available.
	const UINT32 WM_JSON_ARRIVED = 0x8001;
	
	/*
		The amount in milliseocnds that a difference in time between what the game reports and what the HUD is
		keeping is considered a desync and the HUD resets its time to be what the game says.
	*/
	const int DESYNC_THRESHOLD = 200;

	// The height of every icon.
	const float ICON_HEIGHT = 128;
}

} // namespace CsgoHud

#endif // CSGO_HUD_UTILS_COMMONCONSTANTS_H