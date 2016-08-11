#include "TestDepthBuffer.h"
#include "video/ScopedViewPort.h"

TestDepthBuffer::TestDepthBuffer(io::FilesystemPtr filesystem, core::EventBusPtr eventBus) :
		Super(filesystem, eventBus) {
}

void TestDepthBuffer::doRender() {
	Super::doRender();
	const int width = _camera.width();
	const int height = _camera.height();
	const GLsizei quadWidth = (GLsizei) (width / 3.0f);
	const GLsizei quadHeight = (GLsizei) (height / 3.0f);
	video::ScopedShader scopedShader(_shadowMapRenderShader);
	video::ScopedViewPort scopedViewport(width - quadWidth, 0, quadWidth, quadHeight);
	core_assert_always(_texturedFullscreenQuad.bind());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthBuffer.getTexture());
	_shadowMapRenderShader.setShadowmap(0);
	glDrawArrays(GL_TRIANGLES, 0, _texturedFullscreenQuad.elements(0));
	_texturedFullscreenQuad.unbind();
	glBindTexture(GL_TEXTURE_2D, 0);
}

core::AppState TestDepthBuffer::onInit() {
	core::AppState state = Super::onInit();
	if (!_shadowMapRenderShader.setup()) {
		Log::error("Failed to init shadowmaprender shader");
		return core::AppState::Cleanup;
	}
	const glm::ivec2& fullscreenQuadIndices = _texturedFullscreenQuad.createFullscreenTexturedQuad();
	_texturedFullscreenQuad.addAttribute(_shadowMapRenderShader.getLocationPos(), fullscreenQuadIndices.x, 3);
	_texturedFullscreenQuad.addAttribute(_shadowMapRenderShader.getLocationTexcoord(), fullscreenQuadIndices.y, 2);
	return state;
}

core::AppState TestDepthBuffer::onCleanup() {
	_texturedFullscreenQuad.shutdown();
	_shadowMapRenderShader.shutdown();
	return Super::onCleanup();
}

int main(int argc, char *argv[]) {
	return core::getApp<TestDepthBuffer>()->startMainLoop(argc, argv);
}
