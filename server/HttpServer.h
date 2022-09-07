#ifndef CSGO_HUD_SERVER_HTTPSERVER_H
#define CSGO_HUD_SERVER_HTTPSERVER_H

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "pch.h"

namespace CsgoHud {

/*
	An HTTP server using Windows HTTP server API to receive game state integration JSON payloads.
*/
class HttpServer final {
	private:
		HANDLE queue;
		std::string
			jsons1, jsons2,
			*currentJsons = &jsons1;
		std::vector<std::chrono::time_point<std::chrono::steady_clock>>
			timestamps1, timestamps2,
			*currentTimestamps = &timestamps1;
		bool bufferSwitch = true;
		bool notificationSent = false;
		std::atomic_bool finished = false;
	public:
		std::mutex mutex;
		
		void run(HWND windowHandle);
		void stop();
		const std::string& getCurrentJsons();
		const std::vector<std::chrono::time_point<std::chrono::steady_clock>>& getCurrentTimestamps();
		void swapBuffers();
};

} // namespace CsgoHud

#endif // CSGO_HUD_SERVER_HTTPSERVER_H