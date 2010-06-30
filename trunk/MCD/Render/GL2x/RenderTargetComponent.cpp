#include "Pch.h"
#include "../RenderTargetComponent.h"
#include "Renderer.inc"
#include "../Camera.h"
#include "../RenderWindow.h"
#include "../Texture.h"
#include "../../Core/Entity/Entity.h"
#include "../../../3Party/glew/glew.h"

namespace MCD {

RenderTargetComponent::RenderTargetComponent()
	: shouldClearColor(true), shouldClearDepth(true)
	, clearColor(0, 1)
	, viewPortLeftTop(0), viewPortWidthHeight(0)
	, window(nullptr), mImpl(0)
{
}

RenderTargetComponent::~RenderTargetComponent()
{
	glDeleteFramebuffers(1, (GLuint*)&mImpl);
}

TexturePtr RenderTargetComponent::createTexture(const GpuDataFormat& format, size_t width, size_t height)
{
	TexturePtr texture = new Texture("");
	if(!texture->create(format, format, width, height, 1, 1, nullptr, 0, 123))
		texture = nullptr;
	return texture;
}

static void setViewPort(RenderTargetComponent& renderTarget)
{
	size_t width;
	size_t height;

	width =
		renderTarget.viewPortWidthHeight.x != 0 ?
			renderTarget.viewPortWidthHeight.x :
		renderTarget.window ?
			renderTarget.window->width() :
		renderTarget.textures[0] ?
			renderTarget.textures[0]->width :
			0;

	height =
		renderTarget.viewPortWidthHeight.y != 0 ?
			renderTarget.viewPortWidthHeight.y :
		renderTarget.window ?
			renderTarget.window->height() :
		renderTarget.textures[0] ?
			renderTarget.textures[0]->height :
			0;

	// Minimized window may resulting zero width height
	if(width * height == 0)
		return;

	glViewport(0, 0, width, height);

	// Also adjust the frustum's aspect ration
	if(renderTarget.cameraComponent)
		renderTarget.cameraComponent->frustum.setAcpectRatio(float(width) / height);
}

void RenderTargetComponent::render()
{
	if(rendererComponent)
		rendererComponent->mImpl.mRenderTargets.push_back(this);
}

void RenderTargetComponent::render(RendererComponent& renderer, bool swapBuffers)
{
	if(!entityToRender)
		return;

	// Clear any GL error
	(void)glGetError();

	size_t textureCount = 0;
	for(; textureCount<textures.size(); ++textureCount)
		if(!textures[textureCount]) break;

	if(window && !textureCount) {
		window->makeActive();
		window->preUpdate();

		setViewPort(*this);

		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(shouldClearColor * GL_COLOR_BUFFER_BIT | shouldClearDepth * GL_DEPTH_BUFFER_BIT);

		renderer.render(*entityToRender, *this);

		if(swapBuffers)
			window->postUpdate();
	}

	// Texture only
	if(!window && textureCount)
	{
		if(mImpl == 0) {
			glDeleteFramebuffers(1, (GLuint*)&mImpl);
			glGenFramebuffers(1, (GLuint*)&mImpl);

			glBindFramebuffer(GL_FRAMEBUFFER_EXT, mImpl);

			for(size_t i=0; i<textureCount; ++i)
			{
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
					GL_TEXTURE_2D, textures[i]->handle, 0/*mipmap level*/);
			}

			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			switch (status) {
				case GL_FRAMEBUFFER_COMPLETE:
					status = status;
				default:
					break;
			}
		}
		else
			glBindFramebuffer(GL_FRAMEBUFFER, mImpl);

		setViewPort(*this);
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		glClear(shouldClearColor * GL_COLOR_BUFFER_BIT | shouldClearDepth * GL_DEPTH_BUFFER_BIT);

		renderer.render(*entityToRender, *this);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	{	// Print GL error if any
		const int err = glGetError();
		if(err != GL_NO_ERROR)
			printf("OpenGl has error 0x%08x\n", err);
	}
}

}	// namespace MCD
