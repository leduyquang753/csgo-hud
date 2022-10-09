#include "pch.h"

#include <memory>
#include <utility>

#include "components/base/Component.h"
#include "resources/CommonResources.h"

#include "components/base/SingleChildComponent.h"

namespace CsgoHud {

// == SingleChildComponent ==

SingleChildComponent::SingleChildComponent(CommonResources &commonResources, std::unique_ptr<Component> &&child):
	Component(commonResources), child(std::move(child))
{}

void SingleChildComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	if (child) paintChild(transform, parentSize);
}

} // namespace CsgoHud