#include "pch.h"

#include <array>
#include <string>

#include "data/Icon.h"
#include "resources/CommonResources.h"

#include "data/IconStorage.h"

using namespace std::string_literals;

namespace CsgoHud {

// == IconStorage ==

void IconStorage::loadIcons(CommonResources &commonResources) {
	const std::array<std::wstring, 52> fileNames = {
		/*
			Weapons in the order corresponding to that in `data\WeaponTypes.cpp` so that lookup can be
			easily done.
		*/
		L"AK-47"s, // Index: 0
		L"AUG"s,
		L"AWP"s,
		L"PP-Bizon"s,
		L"FAMAS"s,
		L"G3SG1"s, // 5
		L"Galil AR"s,
		L"M249"s,
		L"M4A4"s,
		L"M4A1-S"s,
		L"MAC-10"s, // 10
		L"Mag-7"s,
		L"MP5-SD"s,
		L"MP7"s,
		L"MP9"s,
		L"Negev"s, // 15
		L"Nova"s,
		L"P90"s,
		L"Sawed-off"s,
		L"SCAR-20"s,
		L"SG 553"s, // 20
		L"SSG 08"s,
		L"UMP-45"s,
		L"XM1014"s,

		L"CZ75 auto"s,
		L"Desert eagle"s, // 25
		L"Dual Berettas"s,
		L"Five-seveN"s,
		L"Glock-18"s,
		L"P2000"s,
		L"P250"s, // 30
		L"R8 revolver"s,
		L"Tec-9"s,
		L"USP-S"s,

		L"Zeus x27"s,

		L"Decoy grenade"s, // 35
		L"Stun grenade"s,
		L"Frag grenade"s,
		L"Incendiary grenade"s,
		L"Molotov cocktail"s,
		L"Smoke grenade"s, // 40

		// Other icons.
		L"C4"s,
		L"Defuse kit"s,
		L"Kevlar"s,
		L"Full armor"s,
		L"Health"s, // 45
		L"Dead"s,
		L"Explosion"s,
		L"Timer"s,
		L"Fire"s,
		L"X"s, // 50
		L"LED"s // 51
	};

	winrt::com_ptr<IWICImagingFactory> imagingFactory;
	CoCreateInstance(
		CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(imagingFactory.put())
	);

	static const UINT32 SPRITE_SIZE = 2048;
	static const D2D1_SIZE_U SPRITE_SIZE_STRUCT = {SPRITE_SIZE, SPRITE_SIZE};
	static const D2D1_PIXEL_FORMAT SPRITE_FORMAT = {
		.format = DXGI_FORMAT_B8G8R8A8_UNORM, .alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED
	};
	commonResources.renderTarget->CreateCompatibleRenderTarget(
		nullptr, &SPRITE_SIZE_STRUCT,
		&SPRITE_FORMAT, D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
		spriteTarget.put()
	);
	winrt::com_ptr<ID2D1Bitmap> bitmap;

	static const UINT32 PADDING = 10, ICON_HEIGHT = 128;
	UINT32 imageX = PADDING, imageY = PADDING;

	spriteTarget->BeginDraw();
	for (int i = 0; i != fileNames.size(); ++i) {
		const std::wstring path = L"Icons\\"s + fileNames[i] + L".png"s;

		winrt::com_ptr<IWICBitmapDecoder> decoder;
		imagingFactory->CreateDecoderFromFilename(
			path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.put()
		);
		winrt::com_ptr<IWICBitmapFrameDecode> frameDecoder;
		decoder->GetFrame(0, frameDecoder.put());
		winrt::com_ptr<IWICFormatConverter> converter;
		imagingFactory->CreateFormatConverter(converter.put());
		converter->Initialize(
			frameDecoder.get(),
			GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeMedianCut
		);
		auto &icon = icons[i];
		converter->GetSize(&icon.width, &icon.height);
		UINT32 newImageX = imageX + icon.width + PADDING;
		if (newImageX > SPRITE_SIZE) {
			imageY += ICON_HEIGHT + PADDING;
			imageX = PADDING;
			newImageX = imageX + icon.width + PADDING;
		}
		icon.bounds = {imageX, imageY, newImageX, imageY + ICON_HEIGHT};
		icon.floatBounds = {
			static_cast<float>(imageX), static_cast<float>(imageY),
			static_cast<float>(newImageX), static_cast<float>(imageY + ICON_HEIGHT)
		};
		bitmap = nullptr;
		commonResources.renderTarget->CreateBitmapFromWicBitmap(converter.get(), nullptr, bitmap.put());
		spriteTarget->DrawBitmap(
			bitmap.get(), icon.floatBounds, 1, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, nullptr
		);
		imageX = newImageX;
	}
	auto res = spriteTarget->EndDraw();
	spriteTarget->GetBitmap(sprite.put());
}

const Icon& IconStorage::operator[](const int index) const {
	return icons[index];
}

ID2D1Bitmap* IconStorage::getBitmap() const {
	return sprite.get();
}

} // namespace CsgoHud