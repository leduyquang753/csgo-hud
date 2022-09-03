#include <memory>
#include <utility>

#include "pch.h"

#include "components/base/Component.h"
#include "components/base/BagComponent.h"
#include "components/base/PaddedComponent.h"
#include "components/content/FourCornersComponent.h"
#include "components/content/TopBarComponent.h"

#include "components/content/HudComponent.h"

namespace CsgoHud {

struct CommonResources;

// == HudComponent ==

HudComponent::HudComponent(CommonResources &commonResources):
	Component(commonResources), bag(std::make_unique<BagComponent>(commonResources))
{
	bag->children.emplace_back(std::make_unique<FourCornersComponent>(commonResources));
	bag->children.emplace_back(
		std::make_unique<PaddedComponent>(commonResources, 8.f,
			std::make_unique<TopBarComponent>(commonResources)
		)
	);
}

void HudComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	bag->paint(transform, parentSize);
}

} // namespace CsgoHud