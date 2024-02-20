#pragma once
#include "util/base.hpp"
#include <optional>
#include <vector>




struct VertexDescriptor
{
	struct DataType
	{
		u16 gltype;
		u8  count;
	};
	std::vector<DataType> attributes;


	u8  count   (u8 attributeIndex) const { return attributes[attributeIndex].count; }
	u32 size    (u8 attributeIndex) const;
	u32 typeSize(u8 attributeIndex) const;
	u32 offset  (u8 attributeIndex) const;


	u8     attributeCount() const { return attributes.size(); }
	size_t totalSize() 		const;

	static VertexDescriptor defaultVertex() {
		return VertexDescriptor{{  { 0x1401, 1 }  }}; /* 0x1401 = GL_UNSIGNED_BYTE */
	}
};


struct BufferDescriptor
{
	void* 			 data;
	u32   			 count;
	VertexDescriptor vinfo;
};


struct ElementBufferRenderData
{
	u32 count;
	u16 gl_type;
	u16 gl_size;
};


struct Buffer
{
public:
	Buffer() = default;
	
	void create(BufferDescriptor const& info, 		u32 usage);
	void update(BufferDescriptor const& updateInfo, u32 byteOffset); /* [NOTE]: vinfo is unused, as there is no purpose for it. */
	void destroy();


protected:
	u32 m_id;
	BufferDescriptor m_info;

	template<bool usingIBO> friend struct VertexArrayBase;
	template<bool usingIBO> friend struct VertexArray;
	friend struct ShaderStorageBuffer;
};


struct ShaderStorageBuffer
{
public:
	void create(BufferDescriptor const& info, u32 usage);
	void update(u32 byteOffset, BufferDescriptor const& info);
	void destroy();


	void setBindingIndex(u32 binding);
	void clearBindingIndex();
	void bind();
	void unbind();
private:
	Buffer m_base;
	u32    m_bindingPoint;
};




template<bool usingIndexBuffer> struct VertexArrayBase
{
public:
	VertexArrayBase() = default;
	
	void createCommon(Buffer& Vertices);
	void destroy();
	void bind()   const;
	void unbind() const;


protected:
	u32 m_vao, m_vbo;
};


template<bool usingIndexBuffer> struct VertexArray {
	VertexArray() = default;
};

template<> struct VertexArray<false> : VertexArrayBase<false> 
{
public:
	void create(Buffer& Vertices) {
		createCommon(Vertices);
		return;
	}
};

template<> struct VertexArray<true> : VertexArrayBase<true>
{
public:
	void create(Buffer& Vertices, Buffer& Indices);
	ElementBufferRenderData const& getRenderData() const { return m_renderData; }


protected:
	u32 m_ebo;
	ElementBufferRenderData m_renderData;
};