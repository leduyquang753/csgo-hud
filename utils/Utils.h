#ifndef CSGO_HUD_UTILS_UTILS_H
#define CSGO_HUD_UTILS_UTILS_H

#include <string>

namespace CsgoHud {

namespace Utils {
	std::wstring widenString(const std::string &string);
	std::wstring formatTimeAmount(int millis);
}

} // namespace CsgoHud

#endif // CSGO_HUD_UTILS_UTILS_H