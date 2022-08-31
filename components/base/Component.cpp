#include "pch.h"

#include "components/base/Component.h"

namespace CsgoHud {

struct CommonResources;

// == Component ==

Component::Component(CommonResources &commonResources): commonResources(commonResources) {}

} // namespace CsgoHud