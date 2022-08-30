#ifndef CSGO_HUD_RESOURCES_COMMONRESOURCES_H
#define CSGO_HUD_RESOURCES_COMMONRESOURCES_H

#include "pch.h"

namespace CsgoHud {

struct CommonResources final {
	winrt::com_ptr<ID2D1Factory> d2dFactory;
	winrt::com_ptr<ID2D1DCRenderTarget> renderTarget;
	winrt::com_ptr<IDWriteFactory> writeFactory;
};

} // namespace CsgoHud

#endif // CSGO_HUD_RESOURCES_COMMONRESOURCES_H