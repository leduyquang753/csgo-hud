#ifndef CSGO_HUD_RESOURCES_COMMONRESOURCES_H
#define CSGO_HUD_RESOURCES_COMMONRESOURCES_H

#include "pch.h"

#include "events/EventBus.h"
#include "server/HttpServer.h"

namespace CsgoHud {

struct CommonResources final {
	HttpServer httpServer;
	
	winrt::com_ptr<ID2D1Factory> d2dFactory;
	winrt::com_ptr<ID2D1DCRenderTarget> renderTarget;
	winrt::com_ptr<IDWriteFactory7> writeFactory;

	EventBus eventBus;
};

} // namespace CsgoHud

#endif // CSGO_HUD_RESOURCES_COMMONRESOURCES_H