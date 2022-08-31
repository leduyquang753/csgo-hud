#include <memory>
#include <utility>
#include <vector>

#include "pch.h"

#include "components/base/Component.h"

#include "components/base/MultipleChildrenComponent.h"

namespace CsgoHud {

struct CommonResources;

// == MultipleChildrenComponent ==

MultipleChildrenComponent::MultipleChildrenComponent(
	CommonResources &commonResources, std::vector<std::unique_ptr<Component>> &&children
): Component(commonResources), children(std::move(children)) {}

} // namespace CsgoHud