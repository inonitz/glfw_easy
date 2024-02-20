#include "texture.hpp"
#include "awc/opengl.hpp"




void TextureBuffer::create(const TextureBufferDescriptor &inf)
{
	m_info = inf;
	gl()->CreateTextures(GL_TEXTURE_2D, 1, &m_id);
	for(auto& param_pair : m_info.parameters) {
		gl()->TextureParameteri(m_id, param_pair.name, param_pair.val);
	}
	recreateImage(inf.dims);
	return;
}


void TextureBuffer::destroy()
{
	if(m_id != DEFAULT32) { 
		if(m_imageUnit   != DEFAULT32) unbindImage();
		if(m_bindingUnit != DEFAULT32) unbindUnit();
		gl()->DeleteTextures(1, &m_id); 
	}
	return;
}


void TextureBuffer::bindToImage(u32 imgUnit, u8 accessRights)
{
	m_imageUnit = imgUnit;
	
	u32 level = 0;
	accessRights &= 0b11;
	gl()->BindImageTexture(imgUnit, m_id, level, GL_FALSE, level, GL_READ_ONLY + accessRights, m_info.format.internalFmt);
	return;
}


void TextureBuffer::bindToUnit(u32 textureUnit)
{
	m_bindingUnit = textureUnit;
	gl()->BindTextureUnit(m_bindingUnit, m_id);
	return;
}


void TextureBuffer::unbindImage()
{
	gl()->BindImageTexture(m_imageUnit, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
	m_imageUnit = DEFAULT32;
	return;
}


void TextureBuffer::unbindUnit()
{
	gl()->BindTextureUnit(m_bindingUnit, 0);
	m_bindingUnit = DEFAULT32;
	return;
}


// void TextureBuffer::recreateImage(math::vec2u newDims)
// {
// 	info.dims = newDims;
// 	gl()->BindTexture(GL_TEXTURE_2D, id);
// 	gl()->TexImage2D(
// 		GL_TEXTURE_2D, 
// 		0, 
// 		info.format.internalFmt, 
// 		info.dims.x, 
// 		info.dims.y, 
// 		0, 
// 		info.format.layout, 
// 		info.format.gl()->type, 
// 		nullptr
// 	);
// 	gl()->BindTexture(GL_TEXTURE_2D, 0);
// 	return;
// }


void TextureBuffer::recreateImage(math::vec2u newDims)
{
	// unbindImage();
	// unbindUnit();
	m_info.dims = newDims;
	gl()->BindTexture(GL_TEXTURE_2D, m_id);
	gl()->TexImage2D(
		GL_TEXTURE_2D,
		0, 
		m_info.format.internalFmt,
		m_info.dims.x,
		m_info.dims.y,
		0,
		m_info.format.layout,
		m_info.format.gltype,
		m_info.data
	);
	gl()->BindTexture(GL_TEXTURE_2D, 0);
	return;
}