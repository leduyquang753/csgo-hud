#ifndef CSGO_HUD_DATA_ICON_H
#define CSGO_HUD_DATA_ICON_H

namespace CsgoHud {

struct Icon final {
	UINT width, height;
	D2D1_RECT_U bounds;
	D2D1_RECT_F floatBounds;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ICON_H