#ifndef CSGO_HUD_UTILS_COMMONCONSTANTS_H
#define CSGO_HUD_UTILS_COMMONCONSTANTS_H

namespace CsgoHud {

namespace CommonConstants {
	/*
		The amount in milliseocnds that a difference in time between what the game reports and what the HUD is
		keeping is considered a desync and the HUD resets its time to be what the game says.
	*/
	const int DESYNC_THRESHOLD = 200;
}

} // namespace CsgoHud

#endif // CSGO_HUD_UTILS_COMMONCONSTANTS_H