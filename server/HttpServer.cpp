#include <atomic>
#include <chrono>
#include <cstdlib>
#include <locale>
#include <mutex>
#include <new>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <http.h>

#include "utils/CommonConstants.h"

#include "server/HttpServer.h"

namespace CsgoHud {

// == HttpServer ==

static const wchar_t *url = L"http://127.0.0.1:31982/";

void HttpServer::run(const HWND windowHandle) {
	HttpInitialize(HTTPAPI_VERSION_1, HTTP_INITIALIZE_SERVER, nullptr);
	HttpCreateHttpHandle(&queue, 0);
	HttpAddUrl(queue, url, nullptr);

	std::size_t bufferSize = 1 << 20;
	HTTP_REQUEST *request = static_cast<HTTP_REQUEST*>(
		operator new(bufferSize, static_cast<std::align_val_t>(alignof(HTTP_REQUEST)))
	);
	HTTP_REQUEST_ID requestId;
	HTTP_SET_NULL_ID(&requestId);

	std::vector<char> entity(1 << 20);
	std::string json;

	HTTP_RESPONSE response{};
	response.ReasonLength = 0;
	response.pReason = "";

	ULONG bytes;
	bool done = false;
	while (!finished) {
		switch (HttpReceiveHttpRequest(
			queue, requestId, 0, request, static_cast<ULONG>(bufferSize), &bytes, nullptr
		)) {
			case NO_ERROR: {
				if (request->Verb == HttpVerbPOST) {
					if (request->Flags & HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS) {
						json.clear();
						done = false;
						while (!done) {
							switch (HttpReceiveRequestEntityBody(
								queue, request->RequestId,
								HTTP_RECEIVE_REQUEST_ENTITY_BODY_FLAG_FILL_BUFFER,
								entity.data(), static_cast<ULONG>(entity.size()), &bytes, nullptr
							)) {
								case NO_ERROR:
									if (bytes != 0) json.append(entity.data(), bytes);
									break;
								case ERROR_HANDLE_EOF:
									if (bytes != 0) json.append(entity.data(), bytes);
									response.StatusCode = 200; // OK.
									auto result = HttpSendHttpResponse(
										queue, request->RequestId, 0, &response, nullptr, &bytes,
										nullptr, 0, nullptr, nullptr
									);
									mutex.lock();
									*currentJsons += json;
									*currentJsons += '\0';
									currentTimestamps->push_back(std::chrono::steady_clock::now());
									if (!notificationSent) {
										PostMessage(windowHandle, CommonConstants::WM_JSON_ARRIVED, 0, 0);
										notificationSent = true;
									}
									mutex.unlock();
									done = true;
									break;
							}
						}
					} else {
						response.StatusCode = 400; // Bad request.
						HttpSendHttpResponse(
							queue, request->RequestId, 0, &response, nullptr, &bytes, nullptr, 0, nullptr, nullptr
						);
					}
				} else {
					response.StatusCode = 405; // Method not allowed.
					HttpSendHttpResponse(
						queue, request->RequestId, 0, &response, nullptr, &bytes, nullptr, 0, nullptr, nullptr
					);
				}
				HTTP_SET_NULL_ID(&requestId);
				break;
			}
			case ERROR_MORE_DATA: {
				requestId = request->RequestId;
				operator delete(request, static_cast<std::align_val_t>(alignof(HTTP_REQUEST)));
				while (bufferSize < bytes) bufferSize <<= 1;
				request = static_cast<HTTP_REQUEST*>(
					operator new(bufferSize, static_cast<std::align_val_t>(alignof(HTTP_REQUEST)))
				);
				break;
			}
			case ERROR_CONNECTION_INVALID: {
				HTTP_SET_NULL_ID(&requestId);
				break;
			}
		}
	}
	operator delete(request, static_cast<std::align_val_t>(alignof(HTTP_REQUEST)));
}

void HttpServer::stop() {
	finished = true;
	HttpRemoveUrl(queue, url);
	HttpCloseRequestQueue(queue);
	HttpTerminate(HTTP_INITIALIZE_SERVER, nullptr);
}

const std::string& HttpServer::getCurrentJsons() {
	return *currentJsons;
}

const std::vector<std::chrono::time_point<std::chrono::steady_clock>>& HttpServer::getCurrentTimestamps() {
	return *currentTimestamps;
}

void HttpServer::swapBuffers() {
	bufferSwitch = !bufferSwitch;
	currentJsons = bufferSwitch ? &jsons1 : &jsons2;
	currentTimestamps = bufferSwitch ? &timestamps1 : &timestamps2;
	currentJsons->clear();
	currentTimestamps->clear();
	notificationSent = false;
}

} // namespace CsgoHud