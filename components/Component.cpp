#include "pch.h"

#include "components/Component.h"

namespace CsgoHud {

struct CommonResources;

// == Component ==

Component::Component(CommonResources &commonResources): commonResources(commonResources) {}

} // namespace CsgoHud