#include "pch.h"

#include "components/Component.h"

#include "components/BlankComponent.h"

namespace CsgoHud {

struct CommonResources;

// == BlankComponent ==

BlankComponent::BlankComponent(CommonResources &commonResources): Component(commonResources) {}

void BlankComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {}

} // namespace CsgoHud