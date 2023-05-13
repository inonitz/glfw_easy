#pragma once
#include <string_view>
#include <vector>
#include "vec.hpp"




struct ShaderData {
	const char* filepath = nullptr;
	u32 	    type     = DEFAULT32;
	u32 		id       = DEFAULT32;

	ShaderData() : filepath{nullptr}, type{DEFAULT32}, id{DEFAULT32} {}
	ShaderData(const char* fp, u32 glTypeEnum) : filepath{fp}, type{glTypeEnum}, id{DEFAULT32} {}
};


struct BufferData {
	char*  data;
	size_t size;
};


constexpr const char* shaderTypeToString(u32 type);




struct Program 
{
private:
	using shaderContents = std::vector<char>;
	using shaderType 	 = u32;
	using loadedShader   = std::pair<BufferData, shaderType>;
	
	template<bool typeIsShaderMeta> 
	using shaderMetaOrBufferMetaType = typename 
											std::conditional<
												typeIsShaderMeta, 
													std::vector<ShaderData>, 
													std::vector<loadedShader>
												>::type; 

    /* for CREATE_UNIFORM_FUNCTION_DEFINITON macro */
    using array2f = std::array<f32, 2>;
    using array3f = std::array<f32, 3>;
    using array4f = std::array<f32, 4>;
    using array2i = std::array<i32, 2>;
    using array3i = std::array<i32, 3>;
    using array4i = std::array<i32, 4>;
    using array2u = std::array<u32, 2>;
    using array3u = std::array<u32, 3>;
    using array4u = std::array<u32, 4>;


	bool loadShader(ShaderData& init, BufferData const& loadedShader);


	template<bool typeIsShaderMeta> void createFromCommon(shaderMetaOrBufferMetaType<typeIsShaderMeta> const& meta) {
		shaders.resize(meta.size());
		sources.resize(meta.size());
		if constexpr (typeIsShaderMeta) /* simple copy */ { 
			shaders = meta;
		} 
		else { /* we need to init 'shaders' ourselves */
			for(size_t i = 0; i < sources.size(); ++i) {
				/* We wont save the original pointer of the data as we're not taking ownership of it. */
				shaders[i] = { nullptr, meta[i].second };
			}
		}


		for(size_t i = 0; i < shaders.size(); ++i) 
		{
			/* Could combine into one function call with std::conditional, but this is more readable. */
			if constexpr (typeIsShaderMeta) {
				refreshShaderSource(i, meta[i].filepath);
			} else {
				refreshShaderSource(i, meta[i].first);
			}
		}
		return;
	}


public:
	/* 
		createFrom*() will:
			refreshShadersource() for all shaders
		
		compile() will create the actual shader program, which is left to the user.
	*/  
	void createFrom(std::vector<ShaderData>   const& shaders) { createFromCommon<true >(shaders); }
	void createFrom(std::vector<loadedShader> const& buffers) { createFromCommon<false>(buffers); }
    

	/* 
		[NOTE]: 
		There is a 1:1 relation between sources[i] and shaders[i].
		Moreover, shaderID is precisely 'i', which the caller should know
		because he supplied the arguments in that order in createFrom(). 

		These functions ONLY load the shader into the 'sources' vector,
		and initialize the data in 'shaders'. it doesn't create ANY opengl objects.
	*/
	void refreshShaderSource(u32 shaderID, const char* 		 filepath = nullptr);
	void refreshShaderSource(u32 shaderID, BufferData const& buffer);


	void refreshFromFiles() {
		for(size_t i = 0; i < shaders.size(); ++i) { refreshShaderSource(i, shaders[i].filepath); }
		return;
	}
	void refreshFromBuffers() {
		BufferData buf;
		for(size_t i = 0; i < shaders.size(); ++i) {
			buf = { sources[i].data(), sources[i].size() };
			refreshShaderSource(i, buf);
		}
		return;
	}
	
	
	void resizeLocalWorkGroup(u32 shaderID, math::vec3u const& workGroupSize);


	/*
		will use the loaded shader contents to 
		compile each shader, and link it to the program

		will delete the opengl shaders when finished.
	*/
	bool compile();


	void bind();
	void unbind();
	void destroy();


#define CREATE_UNIFORM_FUNCTION_DEFINITON(TypeSpecifier, arg0) [[maybe_unused]] void uniform##TypeSpecifier(std::string_view const& name, arg0);
	CREATE_UNIFORM_FUNCTION_DEFINITON(1f,  f32 v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(1i,  i32 v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(1ui, u32 v)

	CREATE_UNIFORM_FUNCTION_DEFINITON(2f,  array2f const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2i,  array2i const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2ui, array2u const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(3f,  array3f const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3i,  array3i const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3ui, array3u const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(4f,  array4f const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4i,  array4i const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4ui, array4u const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(1fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4fv, f32* v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(1iv, i32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2iv, i32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3iv, i32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4iv, i32* v)

	CREATE_UNIFORM_FUNCTION_DEFINITON(1uiv, u32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(2uiv, u32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(3uiv, u32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(4uiv, u32* v)

	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2fv,   std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3fv,   std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4fv,   std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x3fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x2fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x4fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x2fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x4fv, std::vector<f32> const& v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x3fv, std::vector<f32> const& v)
	
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2fv,   f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3fv,   f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4fv,   f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x3fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x2fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix2x4fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x2fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix3x4fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4x3fv, f32* v)
	CREATE_UNIFORM_FUNCTION_DEFINITON(Matrix4fv, math::mat4f const& v)
#undef CREATE_UNIFORM_FUNCTION_DEFINITON


private:
    u32 m_id = DEFAULT32;
	std::vector<ShaderData> 	shaders;
	std::vector<shaderContents> sources;
};


