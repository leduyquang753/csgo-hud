#include <string>

#include "pch.h"

#include "utils/Utils.h"

namespace CsgoHud {

std::wstring Utils::widenString(const std::string &string) {
	std::wstring wideString(
		MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int>(string.size()), nullptr, 0), 0
	);
	MultiByteToWideChar(
		CP_UTF8, 0, string.data(), static_cast<int>(string.size()),
		wideString.data(), static_cast<int>(wideString.size())
	);
	return wideString;
}

std::wstring Utils::formatTimeAmount(int millis) {
	// Prevent a brief flash of the whole time amount when starting the timer.
	if (millis > 0) --millis;
	const int
		tenths = millis / 100,
		seconds = tenths / 10 % 60;
	std::wstring res;
	if (tenths >= 60 * 10) {
		res += std::to_wstring(tenths / (60 * 10));
		res += L':';
		if (seconds < 10) res += L'0';
	}
	res += std::to_wstring(seconds);
	if (tenths < 60 * 10) {
		if (tenths < 10 * 10) {
			res += L',';
			res += std::to_wstring(tenths % 10);
		}
		res += L'"';
	}
	return res;
}

} // namespace CsgoHud