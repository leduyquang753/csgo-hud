#include <memory>
#include <utility>

#include "pch.h"

#include "components/base/Component.h"
#include "components/base/BagComponent.h"
#include "components/base/PaddedComponent.h"
#include "components/base/SizedComponent.h"
#include "components/base/StackComponent.h"
#include "components/base/StackComponentChild.h"
#include "components/content/BombTimerComponent.h"
#include "components/content/ClockComponent.h"
#include "components/content/FourCornersComponent.h"

#include "components/content/HudComponent.h"

namespace CsgoHud {

struct CommonResources;

// == HudComponent ==

HudComponent::HudComponent(CommonResources &commonResources):
	Component(commonResources), bag(std::make_unique<BagComponent>(commonResources))
{
	bag->children.emplace_back(std::make_unique<FourCornersComponent>(commonResources));
	
	auto topStack = std::make_unique<StackComponent>(
		commonResources,
		StackComponent::AXIS_VERTICAL, StackComponent::AXIS_INCREASE, 0.5f, StackComponent::MODE_RATIO
	);
	topStack->children.emplace_back(StackComponentChild{
		{80, 28}, {StackComponentChild::MODE_PIXELS, StackComponentChild::MODE_PIXELS},
		0.5f, StackComponentChild::MODE_RATIO,
		0, StackComponentChild::MODE_PIXELS,
		std::make_unique<ClockComponent>(commonResources)
	});
	topStack->children.emplace_back(StackComponentChild{
		{1, 24}, {StackComponentChild::MODE_RATIO, StackComponentChild::MODE_PIXELS},
		0.5f, StackComponentChild::MODE_RATIO,
		0, StackComponentChild::MODE_PIXELS,
		std::make_unique<BombTimerComponent>(commonResources)
	});
	
	bag->children.emplace_back(std::make_unique<PaddedComponent>(commonResources, 8.f, std::move(topStack)));
}

void HudComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	bag->paint(transform, parentSize);
}

} // namespace CsgoHud