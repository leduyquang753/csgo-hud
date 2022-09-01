#include "pch.h"

#include "resources/CommonResources.h"

#include "hud/HudWindow.h"

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
	static const int WINDOW_WIDTH = 1600, WINDOW_HEIGHT = 900;
	windowHandle = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT,
		L"HudWindow",
		L"CSGO HUD",
		WS_POPUP,
		0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
		nullptr, nullptr, appInstance, this
	);
	SetWindowPos(
		windowHandle,
		HWND_TOPMOST,
		(GetSystemMetrics(SM_CXSCREEN) - WINDOW_WIDTH) / 2, (GetSystemMetrics(SM_CYSCREEN) - WINDOW_HEIGHT) / 2,
		0, 0,
		SWP_NOSIZE
	);
	
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, commonResources.d2dFactory.put());
	static const D2D1_RENDER_TARGET_PROPERTIES RENDER_TARGET_PROPERTIES = {
		.type = D2D1_RENDER_TARGET_TYPE_DEFAULT,
		.pixelFormat = {.format = DXGI_FORMAT_B8G8R8A8_UNORM, .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED},
		.dpiX = 0, .dpiY = 0,
		.usage = D2D1_RENDER_TARGET_USAGE_NONE,
		.minLevel = D2D1_FEATURE_LEVEL_DEFAULT
	};
	commonResources.d2dFactory->CreateDCRenderTarget(&RENDER_TARGET_PROPERTIES, commonResources.renderTarget.put());
	
	DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(commonResources.writeFactory.put())
	);
	
	RECT windowSize;
	GetClientRect(windowHandle, &windowSize);
	windowSurface = GetDC(windowHandle);
	renderSurface = CreateCompatibleDC(windowSurface);
	SelectObject(renderSurface, CreateCompatibleBitmap(windowSurface, windowSize.right, windowSize.bottom));
	commonResources.renderTarget->BindDC(renderSurface, &windowSize);

	paint();
	ShowWindow(windowHandle, SW_SHOW);

	SetTimer(windowHandle, 1, 1, nullptr);
}

HudWindow::~HudWindow() {
	DeleteDC(renderSurface);
	ReleaseDC(windowHandle, windowSurface);
}

LRESULT HudWindow::handleWindowMessage(const UINT message, const WPARAM wParam, const LPARAM lParam) {
	switch (message) {
		case WM_TIMER:
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

void HudWindow::paint() {
	RECT windowRect;
	GetWindowRect(windowHandle, &windowRect);
	const int
		windowWidth = windowRect.right - windowRect.left,
		windowHeight = windowRect.bottom - windowRect.top;

	auto &renderTarget = *commonResources.renderTarget;	
	renderTarget.BeginDraw();
	renderTarget.Clear({0, 0, 0, 0});

	if (mainComponent) mainComponent->paint(	
		D2D1::Matrix3x2F::Identity(),
		{static_cast<float>(windowWidth), static_cast<float>(windowHeight)}
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

} // namespace CsgoHud