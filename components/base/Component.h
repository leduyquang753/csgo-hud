#ifndef CSGO_HUD_COMPONENTS_BASE_COMPONENT_H
#define CSGO_HUD_COMPONENTS_BASE_COMPONENT_H

namespace CsgoHud {

struct CommonResources;

class Component {
	protected:
		CommonResources &commonResources;
	public:
		Component(CommonResources &commonResources);
		virtual ~Component() = default;
		virtual void paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) = 0;
};

} // namespace CsgoHud

#endif // CSGO_HUD_COMPONENTS_BASE_COMPONENT_H