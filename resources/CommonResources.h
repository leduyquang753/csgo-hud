#ifndef CSGO_HUD_RESOURCES_COMMONRESOURCES_H
#define CSGO_HUD_RESOURCES_COMMONRESOURCES_H

#include <optional>

#include "pch.h"

#include "data/AllPlayersData.h"
#include "data/IconStorage.h"
#include "data/RoundsData.h"
#include "data/WeaponTypes.h"
#include "events/EventBus.h"
#include "server/HttpServer.h"

namespace CsgoHud {

struct CommonResources final {
	EventBus eventBus;
	
	HttpServer httpServer;
	
	winrt::com_ptr<ID2D1Factory7> d2dFactory;
	winrt::com_ptr<ID2D1DeviceContext6> renderTarget;
	winrt::com_ptr<IDWriteFactory7> writeFactory;

	IconStorage icons;

	WeaponTypes weaponTypes;
	AllPlayersData players{weaponTypes, eventBus};
	RoundsData rounds{eventBus};
};

} // namespace CsgoHud

#endif // CSGO_HUD_RESOURCES_COMMONRESOURCES_H