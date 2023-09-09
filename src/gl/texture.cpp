#include "texture.hpp"
#include <glad/gl.h>




void TextureBuffer::create(const TextureBufferDescriptor &inf)
{
	info = inf;
	glCreateTextures(GL_TEXTURE_2D, 1, &id);
	for(auto& param_pair : info.parameters) {
		glTextureParameteri(id, param_pair.name, param_pair.val);
	}
	recreateImage(inf.dims);
	return;
}


void TextureBuffer::destroy()
{
	if(id != DEFAULT32) { 
		if(imageUnit   != DEFAULT32) unbindImage();
		if(bindingUnit != DEFAULT32) unbindUnit();
		glDeleteTextures(1, &id); 
	}
	return;
}


void TextureBuffer::bindToImage(u32 imgUnit, u8 accessRights)
{
	imageUnit = imgUnit;
	
	u32 level = 0;
	accessRights &= 0b11;
	glBindImageTexture(imgUnit, id, level, GL_FALSE, level, GL_READ_ONLY + accessRights, info.format.internalFmt);
	return;
}


void TextureBuffer::bindToUnit(u32 textureUnit)
{
	bindingUnit = textureUnit;
	glBindTextureUnit(bindingUnit, id);
	return;
}


void TextureBuffer::unbindImage()
{
	glBindImageTexture(imageUnit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	imageUnit = DEFAULT32;
	return;
}


void TextureBuffer::unbindUnit()
{
	glBindTextureUnit(bindingUnit, 0);
	bindingUnit = DEFAULT32;
	return;
}


// void TextureBuffer::recreateImage(math::vec2u newDims)
// {
// 	info.dims = newDims;
// 	glBindTexture(GL_TEXTURE_2D, id);
// 	glTexImage2D(
// 		GL_TEXTURE_2D, 
// 		0, 
// 		info.format.internalFmt, 
// 		info.dims.x, 
// 		info.dims.y, 
// 		0, 
// 		info.format.layout, 
// 		info.format.gltype, 
// 		nullptr
// 	);
// 	glBindTexture(GL_TEXTURE_2D, 0);
// 	return;
// }


void TextureBuffer::recreateImage(math::vec2u newDims)
{
	// unbindImage();
	// unbindUnit();
	info.dims = newDims;
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(
		GL_TEXTURE_2D,
		0, 
		info.format.internalFmt,
		info.dims.x,
		info.dims.y,
		0,
		info.format.layout,
		info.format.gltype,
		info.data
	);
	glBindTexture(GL_TEXTURE_2D, 0);
	return;
}