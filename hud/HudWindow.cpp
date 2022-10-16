#include "pch.h"

#include <string_view>

#include "events/KeyEventHook.h"
#include "resources/CommonResources.h"
#include "utils/CommonConstants.h"
#include "utils/Utils.h"

#include "hud/HudWindow.h"

using namespace std::string_view_literals;

namespace CsgoHud {

// == HudWindow ==

LRESULT CALLBACK HudWindow::receiveWindowMessage(
	const HWND windowHandle, const UINT message, const WPARAM wParam, const LPARAM lParam
) {
	if (message == WM_CREATE) {
		SetWindowLongPtr(
			windowHandle, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams)
		);
		return 0;
	} else {
		auto hudWindow = reinterpret_cast<HudWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
		return hudWindow
			? hudWindow->handleWindowMessage(message, wParam, lParam)
			: DefWindowProc(windowHandle, message, wParam, lParam);
	}
}

void HudWindow::preInitialize(const HINSTANCE appInstance) {
	static const WNDCLASSEX WINDOW_CLASS = {
		.cbSize = sizeof(WNDCLASSEX),
		.lpfnWndProc = receiveWindowMessage,
		.hInstance = appInstance,
		.lpszClassName = L"HudWindow"
	};
	RegisterClassEx(&WINDOW_CLASS);
}

HudWindow::HudWindow(const HINSTANCE appInstance, CommonResources &commonResources):
	commonResources(commonResources)
{
	const auto &configuration = commonResources.configuration;
	windowHandle = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT,
		L"HudWindow",
		L"CSGO HUD",
		WS_POPUP,
		0, 0, configuration.windowWidth, configuration.windowHeight,
		nullptr, nullptr, appInstance, this
	);
	SetWindowPos(
		windowHandle,
		HWND_TOPMOST,
		(GetSystemMetrics(SM_CXSCREEN) - configuration.windowWidth) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - configuration.windowHeight) / 2 + 11,
		0, 0,
		SWP_NOSIZE
	);
	
	D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory7),
		reinterpret_cast<void**>(commonResources.d2dFactory.put())
	);
	static const D2D1_RENDER_TARGET_PROPERTIES RENDER_TARGET_PROPERTIES = {
		.type = D2D1_RENDER_TARGET_TYPE_DEFAULT,
		.pixelFormat = {.format = DXGI_FORMAT_B8G8R8A8_UNORM, .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED},
		.dpiX = 0, .dpiY = 0,
		.usage = D2D1_RENDER_TARGET_USAGE_NONE,
		.minLevel = D2D1_FEATURE_LEVEL_DEFAULT
	};
	winrt::com_ptr<ID2D1DCRenderTarget> dcRenderTarget;
	commonResources.d2dFactory->CreateDCRenderTarget(&RENDER_TARGET_PROPERTIES, dcRenderTarget.put());
	dcRenderTarget->QueryInterface(commonResources.renderTarget.put());
	
	DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory7),
		reinterpret_cast<IUnknown**>(commonResources.writeFactory.put())
	);
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteRenderingParams> renderingParams;
	winrt::com_ptr<IDWriteRenderingParams2> renderingParams2;
	writeFactory.CreateMonitorRenderingParams(
		MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), renderingParams.put()
	);
	renderingParams->QueryInterface(renderingParams2.put());
	winrt::com_ptr<IDWriteRenderingParams2> customRenderingParams;
	writeFactory.CreateCustomRenderingParams(
		renderingParams2->GetGamma(),
		renderingParams2->GetEnhancedContrast(),
		renderingParams2->GetGrayscaleEnhancedContrast(),
		renderingParams2->GetClearTypeLevel(),
		renderingParams2->GetPixelGeometry(),
		renderingParams2->GetRenderingMode(),
		DWRITE_GRID_FIT_MODE_DISABLED,
		customRenderingParams.put()
	);
	commonResources.renderTarget->SetTextRenderingParams(customRenderingParams.get());
	
	RECT windowSize;
	GetClientRect(windowHandle, &windowSize);
	windowSurface = GetDC(windowHandle);
	renderSurface = CreateCompatibleDC(windowSurface);
	SelectObject(renderSurface, CreateCompatibleBitmap(windowSurface, windowSize.right, windowSize.bottom));
	dcRenderTarget->BindDC(renderSurface, &windowSize);

	commonResources.icons.loadIcons(commonResources);

	paint();
	ShowWindow(windowHandle, SW_SHOW);

	SetTimer(windowHandle, 1, 1, nullptr);

	KeyEventHook::registerHook(
		commonResources.eventBus,
		Utils::parseKeyCode(
			commonResources.configuration.keybindings["toggleKeybindings"sv].value().get_string().value()
		)
	);
}

HudWindow::~HudWindow() {
	KeyEventHook::unregisterHook();
	
	DeleteDC(renderSurface);
	ReleaseDC(windowHandle, windowSurface);
}

LRESULT HudWindow::handleWindowMessage(const UINT message, const WPARAM wParam, const LPARAM lParam) {
	switch (message) {
		case CommonConstants::WM_JSON_ARRIVED:
			tick();
			return 0;
		case WM_TIMER:
			tick();
			paint();
			return 0;
		case WM_CLOSE:
			KillTimer(windowHandle, 1);
			DestroyWindow(windowHandle);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(windowHandle, message, wParam, lParam);
	}
}

void HudWindow::tick() {
	int &time = commonResources.time;
	const auto now = std::chrono::steady_clock::now();
	if (time == -1) {
		firstTick = now;
		time = 0;
	} else {
		int lastTickTime = time;
		auto &eventBus = commonResources.eventBus;
		auto &httpServer = commonResources.httpServer;
		httpServer.mutex.lock();
		const auto &jsons = httpServer.getCurrentJsons();
		const auto &timestamps = httpServer.getCurrentTimestamps();
		const auto &startLengths = httpServer.getCurrentStartLengths();
		httpServer.swapBuffers();
		httpServer.mutex.unlock();
		auto currentStartLength = startLengths.begin();
		for (const auto &timestamp : timestamps) {
			if (currentStartLength->second != 0) {
				time = static_cast<int>(
					std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - firstTick).count()
				);
				eventBus.notifyTimeEvent(time - lastTickTime);
				lastTickTime = time;
				
				JSON::dom::element jsonDocument = jsonParser.parse(
					jsons.c_str() + currentStartLength->first,
					currentStartLength->second, currentStartLength->second + JSON::SIMDJSON_PADDING
				);
				JSON::dom::object json = jsonDocument.get_object();
				eventBus.notifyDataEvent("", json);
				for (auto field : json) {
					JSON::dom::object subJson = field.value.get_object();
					eventBus.notifyDataEvent(std::string(field.key), subJson);
				}
			}
			++currentStartLength;
		}
		time = static_cast<int>(
			std::chrono::duration_cast<std::chrono::milliseconds>(now - firstTick).count()
		);
		eventBus.notifyTimeEvent(time - lastTickTime);
	}
}

void HudWindow::paint() {
	commonResources.grenades.purgeExpiredEntities();
	
	RECT windowRect;
	GetWindowRect(windowHandle, &windowRect);
	const int
		windowWidth = windowRect.right - windowRect.left,
		windowHeight = windowRect.bottom - windowRect.top;

	auto &renderTarget = *commonResources.renderTarget;	
	renderTarget.BeginDraw();
	renderTarget.Clear({0, 0, 0, 0});

	const auto &configuration = commonResources.configuration;
	if (mainComponent) mainComponent->paint(
		D2D1::Matrix3x2F::Scale({configuration.hudScaling, configuration.hudScaling}, {0, 0}),
		{configuration.hudWidth, configuration.hudHeight}
	);

	renderTarget.EndDraw();
	POINT windowCorner = {windowRect.left, windowRect.top};
	SIZE windowSize = {windowWidth, windowHeight};
	POINT zeroPoint = {0, 0};
	static BLENDFUNCTION blendFunction = {
		.BlendOp = AC_SRC_OVER,
		.BlendFlags = 0,
		.SourceConstantAlpha = 255,
		.AlphaFormat = AC_SRC_ALPHA
	};
	UpdateLayeredWindow(
		windowHandle,
		windowSurface, &windowCorner, &windowSize,
		renderSurface, &zeroPoint,
		0,
		&blendFunction,
		ULW_ALPHA
	);
}

HWND HudWindow::getWindowHandle() {
	return windowHandle;
}

} // namespace CsgoHud