#ifndef CSGO_HUD_DATA_GUNDATA_H
#define CSGO_HUD_DATA_GUNDATA_H

namespace CsgoHud {

struct GunData final {
	int type;
	int roundsInClip;
	int clipSize;
	int spareRounds;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_GUNDATA_H