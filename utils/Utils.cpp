#include "pch.h"

#include <string>
#include <string_view>

#include "data/ConfigurationData.h"

#include "utils/Utils.h"

namespace CsgoHud {

std::wstring Utils::widenString(std::string_view string) {
	std::wstring wideString(
		MultiByteToWideChar(CP_UTF8, 0, string.data(), static_cast<int>(string.size()), nullptr, 0), 0
	);
	MultiByteToWideChar(
		CP_UTF8, 0, string.data(), static_cast<int>(string.size()),
		wideString.data(), static_cast<int>(wideString.size())
	);
	return wideString;
}

std::wstring Utils::formatTimeAmount(int millis, const ConfigurationData &configuration) {
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
			res += configuration.formatting.decimalSeparatorComma ? L',' : L'.';
			res += std::to_wstring(tenths % 10);
		}
		if (configuration.formatting.showSecondsSign) res += L'"';
	}
	return res;
}

std::wstring Utils::formatMoneyAmount(const int money, const ConfigurationData &configuration) {
	std::wstring res;
	if (!configuration.formatting.dollarSignAfter) {
		res += L'$';
		if (configuration.formatting.dollarSignWithSpace) res += L' ';
	}
	res += std::to_wstring(money);
	if (configuration.formatting.dollarSignAfter) {
		if (configuration.formatting.dollarSignWithSpace) res += L' ';
		res += L'$';
	}
	return res;
}

D2D_VECTOR_3F Utils::parseVector(std::string_view string) {
	const auto leftComma = string.find(','), rightComma = string.rfind(',');
	return {
		.x = std::stof(std::string(string.substr(0, leftComma))),
		.y = std::stof(std::string(string.substr(leftComma + 2, rightComma - leftComma - 2))),
		.z = std::stof(std::string(string.substr(rightComma + 2, string.size() - rightComma - 2)))
	};
}

DWORD Utils::parseKeyCode(std::string_view string) {
	return std::stol(std::string(string), nullptr, 16);
}

} // namespace CsgoHud