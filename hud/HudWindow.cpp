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
	auto hudWindow = reinterpret_cast<HudWindow*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
	return hudWindow
		? hudWindow->handleWindowMessage(message, wParam, lParam)
		: DefWindowProc(windowHandle, message, wParam, lParam);
}

void HudWindow::preInitialize(const HINSTANCE appInstance) {
	static const WNDCLASSEX WINDOW_CLASS = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = receiveWindowMessage,
		.hInstance = appInstance,
		.hCursor = LoadCursor(nullptr, IDC_ARROW),
		.lpszClassName = L"HudWindow"
	};
	RegisterClassEx(&WINDOW_CLASS);
}

HudWindow::HudWindow(const HINSTANCE appInstance, CommonResources &commonResources):
	commonResources(commonResources)
{
	const auto &configuration = commonResources.configuration;
	windowHandle = CreateWindowEx(
		WS_EX_NOREDIRECTIONBITMAP | WS_EX_LAYERED | WS_EX_TRANSPARENT,
		L"HudWindow", L"CSGO HUD",
		WS_POPUP,
		0, 0, configuration.windowWidth, configuration.windowHeight,
		nullptr, nullptr, appInstance, this
	);
	SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowPos(
		windowHandle,
		HWND_TOPMOST,
		configuration.startingPositionCenter
			? (GetSystemMetrics(SM_CXSCREEN) - configuration.windowWidth) / 2 + configuration.windowOffsetX
			: configuration.windowOffsetX,
		configuration.startingPositionCenter
			? (GetSystemMetrics(SM_CYSCREEN) - configuration.windowHeight) / 2 + configuration.windowOffsetY
			: configuration.windowOffsetY,
		0, 0,
		SWP_NOSIZE
	);
	SetClassLongPtr(
		windowHandle, GCLP_HICON, reinterpret_cast<LONG_PTR>(LoadIcon(appInstance, MAKEINTRESOURCE(1001)))
	);
	
	winrt::com_ptr<ID3D11Device> d3dDevice0;
	winrt::com_ptr<ID3D11DeviceContext> d3dDeviceContext0;
	D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr, 0, D3D11_SDK_VERSION,
		d3dDevice0.put(), nullptr, d3dDeviceContext0.put()
	);
	winrt::com_ptr<ID3D11Device5> d3dDevice = d3dDevice0.as<ID3D11Device5>();
	winrt::com_ptr<ID3D11DeviceContext4> d3dDeviceContext = d3dDeviceContext0.as<ID3D11DeviceContext4>();
	winrt::com_ptr<IDXGIDevice4> dxgiDevice = d3dDevice.as<IDXGIDevice4>();
	winrt::com_ptr<IDXGIFactory2> dxgiFactory2;
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(dxgiFactory2), dxgiFactory2.put_void());
	winrt::com_ptr<IDXGIFactory7> dxgiFactory = dxgiFactory2.as<IDXGIFactory7>();
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDescription = {
		.Width = static_cast<UINT>(configuration.windowWidth),
		.Height = static_cast<UINT>(configuration.windowHeight),
		.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
		.SampleDesc = {.Count = 1},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = 2,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED
	};
	winrt::com_ptr<IDXGISwapChain1> swapChain1;
	dxgiFactory->CreateSwapChainForComposition(dxgiDevice.get(), &dxgiSwapChainDescription, nullptr, swapChain1.put());
	swapChain = swapChain1.as<IDXGISwapChain4>();
	winrt::com_ptr<IDCompositionDevice> compositionDevice;
	DCompositionCreateDevice(dxgiDevice.get(), __uuidof(compositionDevice), compositionDevice.put_void());
	compositionDevice->CreateTargetForHwnd(windowHandle, true, compositionTarget.put());
	winrt::com_ptr<IDCompositionVisual> compositionVisual;
	compositionDevice->CreateVisual(compositionVisual.put());
	compositionVisual->SetContent(swapChain.get());
	compositionTarget->SetRoot(compositionVisual.get());
	compositionDevice->Commit();
	
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {D2D1_DEBUG_LEVEL_INFORMATION};
	auto &d2dFactory = commonResources.d2dFactory;
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactoryOptions, d2dFactory.put());
	winrt::com_ptr<ID2D1Device6> d2dDevice;
	d2dFactory->CreateDevice(dxgiDevice.get(), d2dDevice.put());
	auto &d2dDeviceContext = commonResources.renderTarget;
	d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, d2dDeviceContext.put());
	winrt::com_ptr<IDXGISurface2> surface;
	swapChain->GetBuffer(0, __uuidof(surface), surface.put_void());
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = {
		.pixelFormat = {
			.format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
		},
		.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW
	};
	winrt::com_ptr<ID2D1Bitmap1> bitmap;
	d2dDeviceContext->CreateBitmapFromDxgiSurface(surface.get(), bitmapProperties, bitmap.put());
	d2dDeviceContext->SetTarget(bitmap.get());
	
	DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory7),
		reinterpret_cast<IUnknown**>(commonResources.writeFactory.put())
	);
	auto &writeFactory = *commonResources.writeFactory;
	winrt::com_ptr<IDWriteRenderingParams> renderingParams;
	writeFactory.CreateMonitorRenderingParams(
		MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), renderingParams.put()
	);
	winrt::com_ptr<IDWriteRenderingParams2> renderingParams2 = renderingParams.as<IDWriteRenderingParams2>();
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
	d2dDeviceContext->SetTextRenderingParams(customRenderingParams.get());

	commonResources.icons.loadIcons(commonResources);

	ShowWindow(windowHandle, SW_SHOW);

	KeyEventHook::registerHook(
		commonResources.eventBus,
		Utils::parseKeyCode(
			commonResources.configuration.keybindings["toggleKeybindings"sv].value().get_string().value()
		)
	);
}

HudWindow::~HudWindow() {
	KeyEventHook::unregisterHook();
}

LRESULT HudWindow::handleWindowMessage(const UINT message, const WPARAM wParam, const LPARAM lParam) {
	switch (message) {
		case CommonConstants::WM_JSON_ARRIVED:
			tick();
			return 0;
		case WM_CLOSE:
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
				for (const auto &field : json) {
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

	auto &renderTarget = *commonResources.renderTarget;	
	renderTarget.BeginDraw();
	renderTarget.Clear({0.f, 0.f, 0.f, 0.f});

	const auto &configuration = commonResources.configuration;
	if (mainComponent) mainComponent->paint(
		D2D1::Matrix3x2F::Scale({configuration.hudScaling, configuration.hudScaling}, {0, 0}),
		{configuration.hudWidth, configuration.hudHeight}
	);

	renderTarget.EndDraw();
	swapChain->Present(1, 0);
}

void HudWindow::update() {
	tick();
	paint();
}

HWND HudWindow::getWindowHandle() {
	return windowHandle;
}

} // namespace CsgoHud