#include "pch.h"

#include <memory>
#include <utility>

#include "components/base/SingleChildComponent.h"
#include "resources/CommonResources.h"

#include "components/base/PaddedComponent.h"

namespace CsgoHud {

// == PaddedComponent ==

PaddedComponent::PaddedComponent(CommonResources &commonResources, std::unique_ptr<Component> &&child):
	SingleChildComponent(commonResources, std::move(child)),
	padding({0, 0, 0, 0})
{}

PaddedComponent::PaddedComponent(
	CommonResources &commonResources, const float padding, std::unique_ptr<Component> &&child
):
	SingleChildComponent(commonResources, std::move(child)),
	padding({padding, padding, padding, padding})
{}

PaddedComponent::PaddedComponent(
	CommonResources &commonResources,
	const float paddingHorizontal, const float paddingVertical, std::unique_ptr<Component> &&child
):
	SingleChildComponent(commonResources, std::move(child)),
	padding({paddingHorizontal, paddingVertical, paddingHorizontal, paddingVertical})
{}

PaddedComponent::PaddedComponent(
	CommonResources &commonResources,
	const float paddingLeft, const float paddingTop, const float paddingRight, const float paddingBottom,
	std::unique_ptr<Component> &&child
):
	SingleChildComponent(commonResources, std::move(child)),
	padding({paddingLeft, paddingTop, paddingRight, paddingBottom})
{}

void PaddedComponent::paintChild(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	child->paint(
		D2D1::Matrix3x2F::Translation({padding.left, padding.top}) * transform,
		{parentSize.width - padding.left - padding.right, parentSize.height - padding.top - padding.bottom}
	);
}

} // namespace CsgoHud