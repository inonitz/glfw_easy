#pragma once
#include "util/base.hpp"
#include <array>




enum class FramebufferStatus {
	COMPLETE,
	UNDEFINED,
	INCOMPLETE_ATTACHMENT,
	INCOMPLETE_MISSING_ATTACHMENT,
	INCOMPLETE_DRAW_BUFFER,
	INCOMPLETE_READ_BUFFER,
	UNSUPPORTED,
	INCOMPLETE_MULTISAMPLE,
	INCOMPLETE_LAYER_TARGETS
};
typedef FramebufferStatus fboStatus;


enum class FramebufferAttachment {
	COLOR0,
	COLOR1,
	COLOR2,
	COLOR3,
	COLOR4,
	COLOR5,
	COLOR6,
	COLOR7,
	COLOR8,
	COLOR9,
	COLOR10,
	COLOR11,
	COLOR12,
	COLOR13,
	COLOR14,
	COLOR15,
	COLOR16,
	COLOR17,
	COLOR18,
	COLOR19,
	COLOR20,
	COLOR21,
	COLOR22,
	COLOR23,
	COLOR24,
	COLOR25,
	COLOR26,
	COLOR27,
	COLOR28,
	COLOR29,
	COLOR30,
	COLOR31,
	DEPTH,
	STENCIL,
	RENDERBUFFER
};
typedef FramebufferAttachment fboAttach;


enum class FramebufferType {
	READ, /* GL_READ_FRAMEBUFFER */
	DRAW, /* GL_DRAW_FRAMEBUFFER */
	BOTH  /* GL_FRAMEBUFFER 	 */
};
typedef FramebufferType fboType;


struct textureAttachmentDescriptor {
	u32 	  textureID;
	fboAttach attachType;
	u32       mipMapLevel;
};
typedef textureAttachmentDescriptor fboTexAttachment;


struct renderbufferAttachmentDescriptor {
	u32 	  rbID;
	fboAttach attachType;
};
typedef renderbufferAttachmentDescriptor fboRenderbufAttachment;



struct Framebuffer
{
private:
	u32 	id;
	fboType type; // read, draw, both.

public:
	Framebuffer() {}


	void create(fboType target);
	void destroy();


	void attachTexture(fboTexAttachment descriptor);
	void detachTexture(fboAttach attachment);
	void attachRenderBuffer(fboRenderbufAttachment descriptor);
	void detachRenderBuffer(fboAttach attachment);
	void bind();
	void unbind();


	__force_inline u32 glid() const { return id; }


	static fboStatus framebufferStatus(Framebuffer const& fb);

	
	/* 
		readFbo / drawFbo = 
			{ ( readFbo/drawFbo )ID[gl], beginX, beginY, endX, endY } 
			where begin/end define the bounds of a rectangle whose dimensions <= Framebuffers' attachment dimensions.
		whichBuffersToCopyMask = 
			mask of { GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STNECIL_BUFFER_BIT }
		linearOrNearest 	   = 
			which filtering to use for interpolation. (GL_LINEAR || GL_NEAREST)
	*/
	static void      blitFramebuffers(
		std::array<u32, 5> const& readFbo, 
		std::array<u32, 5> const& drawFbo,
		u32 whichBuffersToCopyMask,
		u32 LinearOrNearest
	); 
};


u32 	    fboTypeToGL  (fboType    type);
u32 	    fboAttachToGL(fboAttach  type);
u32 	    fboStatusToGL(fboStatus  status);
fboStatus   glToFboStatus(u32 		 glType);
const char* fboStatusToStr(fboStatus status);