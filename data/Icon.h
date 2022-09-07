#ifndef CSGO_HUD_DATA_ICON_H
#define CSGO_HUD_DATA_ICON_H

#include "pch.h"

namespace CsgoHud {

struct Icon final {
	UINT width, height;
	winrt::com_ptr<ID2D1Effect> source;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ICON_H