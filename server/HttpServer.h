#ifndef CSGO_HUD_SERVER_HTTPSERVER_H
#define CSGO_HUD_SERVER_HTTPSERVER_H

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

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
		std::vector<std::pair<std::size_t, std::size_t>>
			startLengths1, startLengths2,
			*currentStartLengths = &startLengths1;
		std::size_t jsonStart = 0;
		bool bufferSwitch = true;
		bool notificationSent = false;
		std::atomic_bool finished = false;
	public:
		std::mutex mutex;

		void run(HWND windowHandle, int port);
		void stop();
		const std::string& getCurrentJsons();
		const std::vector<std::chrono::time_point<std::chrono::steady_clock>>& getCurrentTimestamps();
		const std::vector<std::pair<std::size_t, std::size_t>> getCurrentStartLengths();
		void swapBuffers();
};

} // namespace CsgoHud

#endif // CSGO_HUD_SERVER_HTTPSERVER_H