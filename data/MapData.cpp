#include "pch.h"

#include <filesystem>
#include <string>
#include <string_view>

#include <shlwapi.h>

#include "resources/CommonResources.h"
#include "utils/Utils.h"

#include "data/MapData.h"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace CsgoHud {

// == MapData ==

MapData::MapData(CommonResources &commonResources): commonResources(commonResources) {
	CoCreateInstance(
		CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(imagingFactory.put())
	);
	commonResources.eventBus.listenToDataEvent("map"s, [this](JSON::dom::object &json) { receiveMapData(json); });
}

void MapData::receiveMapData(JSON::dom::object &json) {
	std::string currentMapInternalName(json["name"sv].value().get_string().value());
	if (currentMapInternalName != mapInternalName) {
		mapInternalName = currentMapInternalName;
		const std::wstring folderPath = L"Minimaps\\"s + Utils::widenString(currentMapInternalName);
		std::wstring fullFolderPath(MAX_PATH, '\0');
		fullFolderPath.resize(GetFullPathName(folderPath.c_str(), MAX_PATH, fullFolderPath.data(), nullptr));
		if (PathFileExists(fullFolderPath.c_str())) {
			const std::wstring imagePath = fullFolderPath + L"\\radar.png"s;
			if (
				PathFileExists(imagePath.c_str())
				&& PathFileExists((fullFolderPath + L"\\Configuration.json"s).c_str())
			) {
				// Load the minimap image.
				winrt::com_ptr<IWICBitmapDecoder> decoder;
				imagingFactory->CreateDecoderFromFilename(
					imagePath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.put()
				);
				winrt::com_ptr<IWICBitmapFrameDecode> frameDecoder;
				decoder->GetFrame(0, frameDecoder.put());
				winrt::com_ptr<IWICFormatConverter> converter;
				imagingFactory->CreateFormatConverter(converter.put());
				converter->Initialize(
					frameDecoder.get(),
					GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0,
					WICBitmapPaletteTypeMedianCut
				);
				converter->GetSize(&mapIntWidth, &mapIntHeight);
				mapWidth = static_cast<float>(mapIntWidth);
				mapHeight = static_cast<float>(mapIntHeight);
				mapImage = nullptr;
				commonResources.renderTarget->CreateBitmapFromWicBitmap(converter.get(), nullptr, mapImage.put());

				// Load the configuration.
				auto configuration = jsonParser.load(
					"Minimaps\\"s + currentMapInternalName + "\\Configuration.json"s
				).value().get_object().value();
				mapName = Utils::widenString(configuration["name"sv].value().get_string());
				auto pair = configuration["topLeft"sv].value().get_array().value();
				leftCoordinate = static_cast<float>(pair.at(0).value().get_double());
				topCoordinate = static_cast<float>(pair.at(1).value().get_double());
				pair = configuration["scale"sv].value().get_array().value();
				const float denominator = static_cast<float>(pair.at(1).value().get_double());
				mapScale
					= static_cast<float>(pair.at(0).value().get_double()) / (denominator == 0 ? 1.f : denominator);
				auto optionalLowerLevel = configuration["lowerLevel"sv];
				hasLowerLevel = !optionalLowerLevel.error();
				if (hasLowerLevel) {
					auto lowerLevel = optionalLowerLevel.value().get_object().value();
					pair = lowerLevel["offset"sv].value().get_array().value();
					lowerLevelOffsetX = static_cast<float>(pair.at(0).value().get_double());
					lowerLevelOffsetY = static_cast<float>(pair.at(1).value().get_double());
					levelSeparationHeight
						= static_cast<float>(lowerLevel["separationHeight"sv].value().get_double());
					levelTransitionRange
						= static_cast<float>(lowerLevel["transitionRange"sv].value().get_double());
					const float halfLevelTransitionRange = levelTransitionRange / 2;
					levelSeparationTop = levelSeparationHeight + halfLevelTransitionRange;
					levelSeparationBottom = levelSeparationHeight - halfLevelTransitionRange;
				}
				auto bombsites = configuration["bombsites"sv].value().get_object().value();
				auto readBombsiteCenter = [&bombsites](std::string_view key, D2D1_VECTOR_3F &center) {
					auto point = bombsites[key].value().get_array().value();
					center.x = static_cast<float>(point.at(0).value().get_double());
					center.y = static_cast<float>(point.at(1).value().get_double());
					center.z = static_cast<float>(point.at(2).value().get_double());
				};
				readBombsiteCenter("bombsiteACenter"sv, bombsiteACenter);
				readBombsiteCenter("bombsiteBCenter"sv, bombsiteBCenter);
				bombsiteDistinguishingAxis = bombsites["distinguishingAxis"sv].value().get_string().value()[0];
				bombsiteDistinguishingValue
					= static_cast<float>(bombsites["distinguishingValue"sv].value().get_double());
				bombsiteAToPositiveSide = getDistinguishingAxisValue(bombsiteACenter) > bombsiteDistinguishingValue;

				mapAvailable = true;
			} else {
				mapAvailable = false;
			}
		}
	}
}

float MapData::getDistinguishingAxisValue(const D2D1_VECTOR_3F &coordinates) const {
	switch (bombsiteDistinguishingAxis) {
		case 'x':
			return coordinates.x;
		case 'y':
			return coordinates.y;
		default:
			return coordinates.z;
	}
}

} // namespace CsgoHud