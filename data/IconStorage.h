#ifndef CSGO_HUD_DATA_ICONS_H
#define CSGO_HUD_DATA_ICONS_H

#include <array>

#include "pch.h"

#include "data/Icon.h"

namespace CsgoHud {

struct CommonResources;

/*
	A storage of all icon images used in the HUD.

	The images are in bitmap form for two reasons:
	1. Direct2D has, in 2022, very rudimentary support for drawing SVGs.
	2. CS:GO's filled weapon icons look like someone just used the flood fill tool on the outlined icons, creating
	very jagged paths which hurt performance.
*/
class IconStorage final {
	private:
		std::array<Icon, 50> icons;
	public:
		static const int
			INDEX_ZEUS = 34,
			INDEX_C4 = 41,
			INDEX_DEFUSE_KIT = 42,
			INDEX_KEVLAR = 43,
			INDEX_FULL_ARMOR = 44,
			INDEX_HEALTH = 45,
			INDEX_DEAD = 46,
			INDEX_EXPLOSION = 47,
			INDEX_TIMER = 48,
			INDEX_FIRE = 49;
		
		void loadIcons(CommonResources &commonResources);
		const Icon& operator[](int index) const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ICONS_H