#include <array>
#include <string>

#include "pch.h"

#include "data/Icon.h"
#include "resources/CommonResources.h"

#include "data/IconStorage.h"

using namespace std::string_literals;

namespace CsgoHud {

// == IconStorage ==

void IconStorage::loadIcons(CommonResources &commonResources) {
	const std::array<std::wstring, 50> fileNames = {
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
		L"Fire"s // 50
	};

	winrt::com_ptr<IWICImagingFactory> imagingFactory;
	CoCreateInstance(
		CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(imagingFactory.put())
	);

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
		commonResources.renderTarget->CreateEffect(CLSID_D2D1BitmapSource, icon.source.put());
		icon.source->SetValue(D2D1_BITMAPSOURCE_PROP_WIC_BITMAP_SOURCE, converter.get());
	}
}

const Icon& IconStorage::operator[](const int index) const {
	return icons[index];
}

} // namespace CsgoHud