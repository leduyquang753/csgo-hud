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
		using StartLengths = std::vector<std::pair<std::size_t, std::size_t>>;
		using ReceiveTimestamps = std::vector<std::chrono::time_point<std::chrono::steady_clock>>;

		HANDLE queue;
		std::string jsons1, jsons2;
		std::string *currentJsons = &jsons1;
		ReceiveTimestamps timestamps1, timestamps2;
		ReceiveTimestamps *currentTimestamps = &timestamps1;
		StartLengths startLengths1, startLengths2;
		StartLengths *currentStartLengths = &startLengths1;
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