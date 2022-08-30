#include "pch.h"

#include "components/Component.h"
#include "resources/CommonResources.h"

#include "components/PaddedComponent.h"

namespace CsgoHud {

// == PaddedComponent ==

PaddedComponent::PaddedComponent(CommonResources &commonResources):
	Component(commonResources),
	padding({0, 0, 0, 0})
{}

PaddedComponent::PaddedComponent(CommonResources &commonResources, const float padding):
	Component(commonResources),
	padding({padding, padding, padding, padding})
{}

PaddedComponent::PaddedComponent(
	CommonResources &commonResources,
	const float paddingHorizontal, const float paddingVertical
):
	Component(commonResources),
	padding({paddingHorizontal, paddingVertical, paddingHorizontal, paddingVertical})
{}

PaddedComponent::PaddedComponent(
	CommonResources &commonResources,
	const float paddingLeft, const float paddingTop, const float paddingRight, const float paddingBottom
):
	Component(commonResources),
	padding({paddingLeft, paddingTop, paddingRight, paddingBottom})
{}

void PaddedComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	if (!child) return;
	child->paint(
		transform * D2D1::Matrix3x2F::Translation({padding.left, padding.top}),
		{parentSize.width - padding.left - padding.right, parentSize.height - padding.top - padding.bottom}
	);
}

} // namespace CsgoHud