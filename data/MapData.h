#ifndef CSGO_HUD_DATA_MAPDATA_H
#define CSGO_HUD_DATA_MAPDATA_H

#include <string>

namespace CsgoHud {

struct CommonResources;

/*
	Contains information about the map being played.
*/
class MapData final {
	private:
		CommonResources &commonResources;
		JSON::dom::parser jsonParser;
		
		void receiveMapData(JSON::dom::object &json);
	public:
		std::string mapInternalName;
		bool mapAvailable = false;
		
		std::wstring mapName;
		
		winrt::com_ptr<IWICImagingFactory> imagingFactory;
		winrt::com_ptr<ID2D1Bitmap> mapImage;
		UINT mapIntWidth, mapIntHeight;
		float mapWidth, mapHeight;

		float leftCoordinate, topCoordinate, mapScale;
		bool hasLowerLevel;
		float
			lowerLevelOffsetX, lowerLevelOffsetY,
			levelSeparationHeight, levelSeparationTop, levelSeparationBottom,
			levelTransitionRange;

		D2D1_VECTOR_3F bombsiteACenter, bombsiteBCenter;
		char bombsiteDistinguishingAxis;
		float bombsiteDistinguishingValue;
		bool bombsiteAToPositiveSide;

		MapData(CommonResources &commonResources);
		float getDistinguishingAxisValue(const D2D1_VECTOR_3F &coordinates) const;
};

} // namespace CsgoHud

#endif