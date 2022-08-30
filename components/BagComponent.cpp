#include "pch.h"

#include "components/Component.h"

#include "components/BagComponent.h"

namespace CsgoHud {

struct CommonResources;

// == BagComponent ==

BagComponent::BagComponent(CommonResources &commonResources): Component(commonResources) {}

void BagComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	for (auto &child : children) child->paint(transform, parentSize);
}

} // namespace CsgoHud