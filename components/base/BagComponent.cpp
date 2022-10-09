#include "pch.h"

#include <memory>
#include <utility>
#include <vector>

#include "components/base/Component.h"
#include "components/base/MultipleChildrenComponent.h"

#include "components/base/BagComponent.h"

namespace CsgoHud {

struct CommonResources;

// == BagComponent ==

BagComponent::BagComponent(CommonResources &commonResources, std::vector<std::unique_ptr<Component>> &&children):
	MultipleChildrenComponent(commonResources, std::move(children))
{}

void BagComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	for (auto &child : children) if (child != nullptr) child->paint(transform, parentSize);
}

} // namespace CsgoHud