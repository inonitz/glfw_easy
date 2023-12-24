#include "texture.hpp"
#include "awc/opengl.hpp"




void TextureBuffer::create(const TextureBufferDescriptor &inf)
{
	info = inf;
	gl::cinst()->CreateTextures(GL_TEXTURE_2D, 1, &id);
	for(auto& param_pair : info.parameters) {
		gl::cinst()->TextureParameteri(id, param_pair.name, param_pair.val);
	}
	recreateImage(inf.dims);
	return;
}


void TextureBuffer::destroy()
{
	if(id != DEFAULT32) { 
		if(imageUnit   != DEFAULT32) unbindImage();
		if(bindingUnit != DEFAULT32) unbindUnit();
		gl::cinst()->DeleteTextures(1, &id); 
	}
	return;
}


void TextureBuffer::bindToImage(u32 imgUnit, u8 accessRights)
{
	imageUnit = imgUnit;
	
	u32 level = 0;
	accessRights &= 0b11;
	gl::cinst()->BindImageTexture(imgUnit, id, level, GL_FALSE, level, GL_READ_ONLY + accessRights, info.format.internalFmt);
	return;
}


void TextureBuffer::bindToUnit(u32 textureUnit)
{
	bindingUnit = textureUnit;
	gl::cinst()->BindTextureUnit(bindingUnit, id);
	return;
}


void TextureBuffer::unbindImage()
{
	gl::cinst()->BindImageTexture(imageUnit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	imageUnit = DEFAULT32;
	return;
}


void TextureBuffer::unbindUnit()
{
	gl::cinst()->BindTextureUnit(bindingUnit, 0);
	bindingUnit = DEFAULT32;
	return;
}


// void TextureBuffer::recreateImage(math::vec2u newDims)
// {
// 	info.dims = newDims;
// 	gl::cinst()->BindTexture(GL_TEXTURE_2D, id);
// 	gl::cinst()->TexImage2D(
// 		GL_TEXTURE_2D, 
// 		0, 
// 		info.format.internalFmt, 
// 		info.dims.x, 
// 		info.dims.y, 
// 		0, 
// 		info.format.layout, 
// 		info.format.gl::cinst()->type, 
// 		nullptr
// 	);
// 	gl::cinst()->BindTexture(GL_TEXTURE_2D, 0);
// 	return;
// }


void TextureBuffer::recreateImage(math::vec2u newDims)
{
	// unbindImage();
	// unbindUnit();
	info.dims = newDims;
	gl::cinst()->BindTexture(GL_TEXTURE_2D, id);
	gl::cinst()->TexImage2D(
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
	gl::cinst()->BindTexture(GL_TEXTURE_2D, 0);
	return;
}