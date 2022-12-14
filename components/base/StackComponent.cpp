#include "pch.h"

#include <vector>

#include "components/base/Component.h"
#include "components/base/StackComponentChild.h"
#include "resources/CommonResources.h"

#include "components/base/StackComponent.h"

namespace CsgoHud {

// == StackComponent ==

StackComponent::StackComponent(
	CommonResources &commonResources,
	const bool axis, const bool axisDirection, const float axisPosition, const bool axisPositionMode,
	std::vector<StackComponentChild> &&children
):
	Component(commonResources),
	axis(axis), axisDirection(axisDirection), axisPosition(axisPosition), axisPositionMode(axisPositionMode),
	children(std::move(children))
{}

void StackComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	const float baseOffset = axisPositionMode
		? (axis ? parentSize.width : parentSize.height) * axisPosition
		: axisPosition;
	float paintPosition = axisDirection ? (axis ? parentSize.height : parentSize.width) : 0;
	for (auto &child : children) {
		const D2D1_SIZE_F effectiveSize = {
			.width = child.sizeMode.x ? parentSize.width * child.size.width : child.size.width,
			.height = child.sizeMode.y ? parentSize.height * child.size.height : child.size.height
		};
		if (axisDirection) paintPosition -= axis ? effectiveSize.height : effectiveSize.width;
		if (child.component != nullptr) {
			if (axis) {
				child.component->paint(
					D2D1::Matrix3x2F::Translation({
						.width
							= baseOffset
							+ (child.offsetMode ? parentSize.width * child.offset : child.offset)
							- (child.anchorMode ? effectiveSize.width * child.anchor : child.anchor),
						.height = paintPosition
					}) * transform,
					effectiveSize
				);
			} else {
				child.component->paint(
					D2D1::Matrix3x2F::Translation({
						.width = paintPosition,
						.height
							= baseOffset
							+ (child.offsetMode ? parentSize.height * child.offset : child.offset)
							- (child.anchorMode ? effectiveSize.height * child.anchor : child.anchor)
					}) * transform,
					effectiveSize
				);
			}
		}
		if (!axisDirection) paintPosition += axis ? effectiveSize.height : effectiveSize.width;
	}
}

} // namespace CsgoHud