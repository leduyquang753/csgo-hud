#ifndef CSGO_HUD_RESOURCES_COMMONRESOURCES_H
#define CSGO_HUD_RESOURCES_COMMONRESOURCES_H

#include <optional>

#include "data/AllPlayersData.h"
#include "data/IconStorage.h"
#include "data/BombData.h"
#include "data/GrenadesData.h"
#include "data/MapData.h"
#include "data/RoundsData.h"
#include "data/WeaponTypes.h"
#include "events/EventBus.h"
#include "server/HttpServer.h"

namespace CsgoHud {

struct CommonResources final {
	int time = -1;
	
	EventBus eventBus;
	
	HttpServer httpServer;
	
	winrt::com_ptr<ID2D1Factory7> d2dFactory;
	winrt::com_ptr<ID2D1DeviceContext6> renderTarget;
	winrt::com_ptr<IDWriteFactory7> writeFactory;

	IconStorage icons;

	WeaponTypes weaponTypes;
	AllPlayersData players{*this};
	MapData map{*this};
	BombData bomb{*this};
	RoundsData rounds{eventBus};
	GrenadesData grenades{*this};
};

} // namespace CsgoHud

#endif // CSGO_HUD_RESOURCES_COMMONRESOURCES_H