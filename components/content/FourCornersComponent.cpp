#include "pch.h"

#include "components/base/Component.h"
#include "resources/CommonResources.h"

#include "components/content/FourCornersComponent.h"

namespace CsgoHud {

// == FourCornersComponent ==

FourCornersComponent::FourCornersComponent(CommonResources &commonResources): Component(commonResources) {
	commonResources.renderTarget->CreateSolidColorBrush({0, 0, 0, 0.5}, brush.put());
	commonResources.eventBus.listenToKeyEvent('C', [this](){ onVisibilityToggle(); });
}

void FourCornersComponent::onVisibilityToggle() {
	shown = !shown;
}

void FourCornersComponent::paint(const D2D1::Matrix3x2F &transform, const D2D1_SIZE_F &parentSize) {
	if (!shown) return;
	auto &renderTarget = *commonResources.renderTarget;
	renderTarget.SetTransform(transform);
	renderTarget.FillRectangle({0, 0, 4, 16}, brush.get());
	renderTarget.FillRectangle({4, 0, 16, 4}, brush.get());
	renderTarget.FillRectangle({0, parentSize.height-16, 4, parentSize.height}, brush.get());
	renderTarget.FillRectangle({4, parentSize.height-4, 16, parentSize.height}, brush.get());
	renderTarget.FillRectangle({parentSize.width-4, 0, parentSize.width, 16}, brush.get());
	renderTarget.FillRectangle({parentSize.width-16, 0, parentSize.width-4, 4}, brush.get());
	renderTarget.FillRectangle({
		parentSize.width-4, parentSize.height-16,
		parentSize.width, parentSize.height
	}, brush.get());
	renderTarget.FillRectangle({
		parentSize.width-16, parentSize.height-4,
		parentSize.width-4, parentSize.height
	}, brush.get());
	renderTarget.SetTransform(D2D1::Matrix3x2F::Identity());
}

} // namespace CsgoHud