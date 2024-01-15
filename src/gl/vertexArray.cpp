#include "vertexArray.hpp"
#include "awc/opengl.hpp"
#include <array>




size_t gltype_size(u16 type)
{
	static constexpr std::array<u16, 10> convert_from = {
		GL_DOUBLE,
		GL_FLOAT,
		GL_HALF_FLOAT,
		GL_UNSIGNED_INT,
		GL_INT,
		GL_UNSIGNED_SHORT,
		GL_SHORT,
		GL_UNSIGNED_BYTE,
		GL_BYTE
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




void Buffer::create(BufferDescriptor const& binfo, u32 usage = GL_STATIC_DRAW)
{
	m_info = binfo;
	gl()->CreateBuffers(1, &m_id);
	gl()->NamedBufferData(m_id, binfo.vinfo.totalSize() * binfo.count, binfo.data, usage);
	return;
}


void Buffer::update(BufferDescriptor const& updateInfo, u32 byteOffset)
{
	// markfmt("gl()->_id: %u\nbyteOffset: %u\ninfo: {\n    .data = %p\n    .count = %u\n}\n",
	// 	m_id,
	// 	byteOffset,
	// 	updateInfo.data,
	// 	updateInfo.count
	// );
	gl()->NamedBufferSubData(m_id, byteOffset, updateInfo.count, updateInfo.data);
	return;
}


void Buffer::destroy() 
{ 
	gl()->DeleteBuffers(1, &m_id);
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
	gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingPoint, m_base.m_id);
	return;
}


void ShaderStorageBuffer::clearBindingIndex()
{
	gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingPoint, 0);
	m_bindingPoint = DEFAULT32;
	return;
}


void ShaderStorageBuffer::bind() {
	// gl()->BindBuffer(GL_SHADER_STORAGE_BUFFER, m_base.m_id);
	gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingPoint, m_base.m_id);
	return;
}


void ShaderStorageBuffer::unbind() {
	// gl()->BindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	gl()->BindBufferBase(GL_SHADER_STORAGE_BUFFER, m_bindingPoint, 0);
	return;
}




void VertexArray::create(Buffer& Vertices, Buffer& Indices)
{
	gl()->CreateVertexArrays(1, &m_vao);
	m_vbo = Vertices.m_id;
	m_ebo = Indices.m_id;

	u32 vboBindingPoint = 0;
	gl()->VertexArrayVertexBuffer(m_vao, vboBindingPoint, Vertices.m_id, 0, Vertices.m_info.vinfo.totalSize());
	gl()->VertexArrayElementBuffer(m_vao, Indices.m_id);

	
	auto& vdesc = Vertices.m_info.vinfo;
	for(size_t i = 0; i < vdesc.attributeCount(); ++i) {
		gl()->EnableVertexArrayAttrib(m_vao, i);
		gl()->VertexArrayAttribFormat(m_vao, i, 
			vdesc.attributes[i].count, 
			vdesc.attributes[i].gltype, 
			GL_FALSE, 
			vdesc.offset(i)
		);
		gl()->VertexArrayAttribBinding(m_vao, i, vboBindingPoint);
	}

	m_renderData = {
		Indices.m_info.count,
		Indices.m_info.vinfo.attributes[0].gltype,
		__scast(u16, Indices.m_info.vinfo.typeSize(0))
	};
	return;
}


void VertexArray::destroy()
{
	gl()->DeleteVertexArrays(1, &m_vao);
	m_vao = DEFAULT32;
	return;
}


void VertexArray::bind()   const { gl()->BindVertexArray(m_vao); }
void VertexArray::unbind() const { gl()->BindVertexArray(0);     }



