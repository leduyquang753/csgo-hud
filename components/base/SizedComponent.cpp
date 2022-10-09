#include "pch.h"

#include <memory>
#include <utility>

#include "components/base/SingleChildComponent.h"
#include "resources/CommonResources.h"

#include "components/base/SizedComponent.h"

namespace CsgoHud {

// == SizedComponent ==

SizedComponent::SizedComponent(
	CommonResources &commonResources,
	const D2D1_SIZE_F &size, const D2D1_POINT_2U &sizeMode,
	const D2D1_POINT_2F &anchor, const D2D1_POINT_2U &anchorMode,
	const D2D1_POINT_2F &offset, const D2D1_POINT_2U &offsetMode,
	std::unique_ptr<Component> &&child
):
	SingleChildComponent(commonResources, std::move(child)),
	size(size), sizeMode(sizeMode), anchor(anchor), anchorMode(anchorMode), offset(offset), offsetMode(offsetMode)
{}

void SizedComponent::paintChild(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	D2D1_SIZE_F effectiveSize = {
		.width = sizeMode.x ? parentSize.width * size.width : size.width,
		.height = sizeMode.y ? parentSize.height * size.height : size.height
	};
	child->paint(
		D2D1::Matrix3x2F::Translation({
			.width
				= (offsetMode.x ? parentSize.width * offset.x : offset.x)
				- (anchorMode.x ? effectiveSize.width * anchor.x : anchor.x),
			.height
				= (offsetMode.y ? parentSize.height * offset.y : offset.y)
				- (anchorMode.y ? effectiveSize.height * anchor.y : anchor.y)
		}) * transform,
		effectiveSize
	);
}

} // namespace CsgoHud