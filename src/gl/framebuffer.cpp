#include "framebuffer.hpp"
#include "awc/opengl.hpp"


const char* fboStatusToStr(fboStatus status)
{
	constexpr const char* convert[9] = {
		"GL_FRAMEBUFFER_COMPLETE",
		"GL_FRAMEBUFFER_UNDEFINED",
		"GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT",
		"GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT",
		"GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER",
		"GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER",
		"GL_FRAMEBUFFER_UNSUPPORTED",
		"GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE",
		"GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS",
	};
	return convert[(u32)status];
}


u32 fboStatusToGL(fboStatus status)
{
	constexpr u32 convert[9] = {
		GL_FRAMEBUFFER_COMPLETE,
		GL_FRAMEBUFFER_UNDEFINED,
		GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
		GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
		GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
		GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
		GL_FRAMEBUFFER_UNSUPPORTED,
		GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
		GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
	};
	return convert[(u32)status];
}


fboStatus glToFboStatus(u32 glType)
{
	constexpr u32 convert[9] = {
		GL_FRAMEBUFFER_COMPLETE,
		GL_FRAMEBUFFER_UNDEFINED,
		GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
		GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
		GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
		GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
		GL_FRAMEBUFFER_UNSUPPORTED,
		GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
		GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
	};

	u32 idx = 0;
	while(convert[idx] != glType && idx < 10) ++idx;
	return (fboStatus)idx;
}


u32 fboAttachToGL(fboAttach type) 
{
	constexpr u32 convert[35] = {
		 GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2,  GL_COLOR_ATTACHMENT3,  GL_COLOR_ATTACHMENT4,
		 GL_COLOR_ATTACHMENT5,  GL_COLOR_ATTACHMENT6,  GL_COLOR_ATTACHMENT7,  GL_COLOR_ATTACHMENT8,  GL_COLOR_ATTACHMENT9,
		GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11, GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14,
		GL_COLOR_ATTACHMENT15, GL_COLOR_ATTACHMENT16, GL_COLOR_ATTACHMENT17, GL_COLOR_ATTACHMENT18, GL_COLOR_ATTACHMENT19,
		GL_COLOR_ATTACHMENT20, GL_COLOR_ATTACHMENT21, GL_COLOR_ATTACHMENT22, GL_COLOR_ATTACHMENT23, GL_COLOR_ATTACHMENT24,
		GL_COLOR_ATTACHMENT25, GL_COLOR_ATTACHMENT26, GL_COLOR_ATTACHMENT27, GL_COLOR_ATTACHMENT28, GL_COLOR_ATTACHMENT29,
		GL_COLOR_ATTACHMENT30, GL_COLOR_ATTACHMENT31, 
		GL_DEPTH_ATTACHMENT,   GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER
	};
	return convert[(u32)type];
}


u32 fboTypeToGL(fboType type) 
{
	constexpr u32 convert[3] = { GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER };
	return convert[(u32)type];
}




void Framebuffer::create(fboType target) 
{
	type = target;
	gl::cinst()->CreateFramebuffers(1, &id);
}


void Framebuffer::destroy() {
	ifcrashdo(id == 0, { 
		printf("Framebuffer::destroy() => Can't Destroy OpenGL Framebuffer Object that hasn't been created\n"); 
	}); 
	gl::cinst()->DeleteFramebuffers(1, &id);
}




void Framebuffer::attachTexture(fboTexAttachment descriptor) 
{
	gl::cinst()->NamedFramebufferTexture(
		id,
		fboAttachToGL(descriptor.attachType),
		descriptor.textureID,
		descriptor.mipMapLevel
	);
	return;
}


void Framebuffer::detachTexture(fboAttach attachment) {
	gl::cinst()->NamedFramebufferTexture(
		id,
		fboAttachToGL(attachment),
		0,
		0
	);
}


void Framebuffer::attachRenderBuffer(fboRenderbufAttachment descriptor)
{
	gl::cinst()->NamedFramebufferRenderbuffer(
		id,
		fboAttachToGL(descriptor.attachType),
		GL_RENDERBUFFER,
		descriptor.rbID
	);
}


void Framebuffer::detachRenderBuffer(fboAttach attachment)
{
	gl::cinst()->NamedFramebufferRenderbuffer(
		id,
		fboAttachToGL(attachment),
		GL_RENDERBUFFER,
		0
	);
}




void Framebuffer::bind()   { gl::cinst()->BindFramebuffer(fboTypeToGL(type), id); }
void Framebuffer::unbind() { gl::cinst()->BindFramebuffer(fboTypeToGL(type),  0); }


fboStatus Framebuffer::framebufferStatus(Framebuffer const& fb)
{
	return glToFboStatus(
		gl::cinst()->CheckNamedFramebufferStatus(
			fb.id, 
			fboTypeToGL(fb.type)
		)
	);
}


void Framebuffer::blitFramebuffers(
	std::array<u32, 5> const& readFbo, 
	std::array<u32, 5> const& drawFbo,
	u32 copyBufMask,
	u32 interp
) {
	gl::cinst()->BlitNamedFramebuffer(readFbo[0], drawFbo[0],
		readFbo[1], readFbo[2], readFbo[3], readFbo[4],
		drawFbo[1], drawFbo[2], drawFbo[3], drawFbo[4],
		copyBufMask,
		interp
	);
}

