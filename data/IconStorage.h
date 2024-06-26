#ifndef CSGO_HUD_DATA_ICONS_H
#define CSGO_HUD_DATA_ICONS_H

#include <array>

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
		winrt::com_ptr<ID2D1BitmapRenderTarget> spriteTarget;
		winrt::com_ptr<ID2D1Bitmap> sprite;
		std::array<Icon, 52> icons;
	public:
		static constexpr int
			INDEX_ZEUS = 34,
			INDEX_DECOY_GRENADE = 35,
			INDEX_STUN_GRENADE = 36,
			INDEX_FRAG_GRENADE = 37,
			INDEX_INCENDIARY_GRENADE = 38,
			INDEX_MOLOTOV_COCKTAIL = 39,
			INDEX_SMOKE_GRENADE = 40,
			INDEX_C4 = 41,
			INDEX_DEFUSE_KIT = 42,
			INDEX_KEVLAR = 43,
			INDEX_FULL_ARMOR = 44,
			INDEX_HEALTH = 45,
			INDEX_DEAD = 46,
			INDEX_EXPLOSION = 47,
			INDEX_TIMER = 48,
			INDEX_FIRE = 49,
			INDEX_X = 50,
			INDEX_LED = 51;

		void loadIcons(CommonResources &commonResources);
		const Icon& operator[](int index) const;
		ID2D1Bitmap* getBitmap() const;
};

} // namespace CsgoHud

#endif // CSGO_HUD_DATA_ICONS_H