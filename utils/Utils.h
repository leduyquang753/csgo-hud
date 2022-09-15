#ifndef CSGO_HUD_UTILS_UTILS_H
#define CSGO_HUD_UTILS_UTILS_H

#include <string>
#include <string_view>

#include "pch.h"

namespace CsgoHud {

namespace Utils {
	std::wstring widenString(std::string_view string);
	std::wstring formatTimeAmount(int millis);
	D2D_VECTOR_3F parseVector(std::string_view string);
}

} // namespace CsgoHud

#endif // CSGO_HUD_UTILS_UTILS_H