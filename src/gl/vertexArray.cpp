#include "vertexArray.hpp"
#include <glbinding/gl/gl.h>
#include <array>




size_t gltype_size(u16 type)
{
	static constexpr std::array<u16, 10> convert_from = {
		__scast(u16, gl::GL_DOUBLE),
		__scast(u16, gl::GL_FLOAT),
		__scast(u16, gl::GL_HALF_FLOAT),
		__scast(u16, gl::GL_UNSIGNED_INT),
		__scast(u16, gl::GL_INT),
		__scast(u16, gl::GL_UNSIGNED_SHORT),
		__scast(u16, gl::GL_SHORT),
		__scast(u16, gl::GL_UNSIGNED_BYTE),
		__scast(u16, gl::GL_BYTE)
	};
	static constexpr std::array<size_t, 10> convert_to = { 8, 4, 2, 4, 4, 2, 2, 1, 1, DEFAULT64 };

	u8 i = 0;
	while(i < 9 && convert_from[i] != type) ++i;
	return convert_to[i];
}


u32 VertexDescriptor::size    (u8 attributeIndex) const
{
	DataType local = attributes[attributeIndex];
	return gltype_size(local.gltype) * local.count;
}


u32 VertexDescriptor::typeSize(u8 attributeIndex) const
{
	return gltype_size(attributes[attributeIndex].gltype);
}


u32 VertexDescriptor::offset  (u8 attributeIndex) const
{
	u32 sum = 0;
	for(size_t i = 0; i < attributeIndex; ++i) {
		sum += size(i);
	}
	return sum;
}


size_t VertexDescriptor::totalSize() const
{
	size_t sum = 0; 
	for(size_t i = 0; i < attributeCount(); ++i) { 
		sum += size(i); 
	} 
	return sum; 
}




void Buffer::create(BufferDescriptor const& binfo, u32 usage = __scast(u32, gl::GL_STATIC_DRAW))
{
	m_info = binfo;
	gl::glCreateBuffers(1, &m_id);
	gl::glNamedBufferData(m_id, binfo.vinfo.totalSize() * binfo.count, binfo.data, gl::GLenum{usage});
	return;
}


void Buffer::update(BufferDescriptor const& updateInfo, u32 byteOffset)
{
	// markfmt("gl::gl_id: %u\nbyteOffset: %u\ninfo: {\n    .data = %p\n    .count = %u\n}\n",
	// 	m_id,
	// 	byteOffset,
	// 	updateInfo.data,
	// 	updateInfo.count
	// );
	gl::glNamedBufferSubData(m_id, byteOffset, updateInfo.count, updateInfo.data);
	return;
}


void Buffer::destroy() 
{ 
	gl::glDeleteBuffers(1, &m_id);
	m_id = DEFAULT32;
	return;
}




void ShaderStorageBuffer::create(BufferDescriptor const& info, u32 usage)
{
	m_base.create(info, usage);
	m_bindingPoint = DEFAULT32;
	return;
}


void ShaderStorageBuffer::update(u32 byteOffset, BufferDescriptor const& info)
{
	m_base.update(info, byteOffset);
	return;
}


void ShaderStorageBuffer::destroy()
{
	unbind();
	clearBindingIndex();
	m_base.destroy();
	return;
}


void ShaderStorageBuffer::setBindingIndex(u32 binding)
{
	m_bindingPoint = binding;
	gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, m_bindingPoint, m_base.m_id);
	return;
}


void ShaderStorageBuffer::clearBindingIndex()
{
	gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, m_bindingPoint, 0);
	m_bindingPoint = DEFAULT32;
	return;
}


void ShaderStorageBuffer::bind() {
	// gl::glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_base.m_id);
	gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, m_bindingPoint, m_base.m_id);
	return;
}


void ShaderStorageBuffer::unbind() {
	// gl::glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	gl::glBindBufferBase(gl::GL_SHADER_STORAGE_BUFFER, m_bindingPoint, 0);
	return;
}


void VertexArray::createCommon(Buffer& Vertices)
{
	u32 vboBindingPoint = 0;
	
	m_vbo = Vertices.m_id;
	gl::glCreateVertexArrays(1, &m_vao);
	gl::glVertexArrayVertexBuffer(m_vao, vboBindingPoint, Vertices.m_id, 0, Vertices.m_info.vinfo.totalSize());


	auto& vdesc = Vertices.m_info.vinfo;
	for(size_t i = 0; i < vdesc.attributeCount(); ++i) 
	{
		gl::glEnableVertexArrayAttrib(m_vao, i);
		gl::glVertexArrayAttribFormat(m_vao, i, 
			vdesc.attributes[i].count, 
			gl::GLenum{vdesc.attributes[i].gltype}, 
			0,
			vdesc.offset(i)
		);
		gl::glVertexArrayAttribBinding(m_vao, i, vboBindingPoint);
	}
	return;	
}


void VertexArray::create(Buffer& Vertices, Buffer& Indices)
{
	m_ebo = Indices.m_id;
	gl::glVertexArrayElementBuffer(m_vao, Indices.m_id);
	createCommon(Vertices);

	m_renderData = {
		Indices.m_info.count,
		Indices.m_info.vinfo.attributes[0].gltype,
		__scast(u16, Indices.m_info.vinfo.typeSize(0))
	};
	return;
}


void VertexArray::destroy()
{
	gl::glDeleteVertexArrays(1, &m_vao);
	m_vao = DEFAULT32;
	return;
}


void VertexArray::bind()   const { gl::glBindVertexArray(m_vao); }
void VertexArray::unbind() const { gl::glBindVertexArray(0);     }