#pragma once
#include "manvec.hpp"
#include "glad/glad.h"
#include <string_view>




#define SHADER_TYPE_VERTEX   ((u8)0)
#define SHADER_TYPE_FRAGMENT ((u8)1)
#define SHADER_TYPE_COMPUTE  ((u8)2)




struct ShaderMetadata 
{
    u8  type;
    u64 srcLength;
    u8* srcPointer;
};


struct Shader 
{
    u32 id;
#ifdef _DEBUG
    ShaderMetadata srcData;
#endif


    Shader(std::string_view const& path, u8 type);
};


struct ShaderProgram 
{
    std::vector<u32>            shaderID; // shader[0] is program ID
#ifdef _DEBUG
    std::vector<ShaderMetadata> shaderDebug;
#endif
    ShaderProgram() = default;
    ~ShaderProgram() = default;


    void create();
    void destroy();


    ShaderProgram& addShader(Shader const& shader);
    ShaderProgram& addShader(std::string_view const& path, u8 shaderType);

    void finalize();
    __force_inline void bind() const   { glUseProgram(shaderID[0]); return; }
    static void unbindCurrentProgram() { glUseProgram(0);           return; }


    [[maybe_unused]] void 	      set1f(std::string_view const& name, f32 v						) 			         {          glUniform1f( glGetUniformLocation( shaderID[0], name.cbegin() ), v  		 			); }
    [[maybe_unused]] void 	      set1i(std::string_view const& name, i32 v						) 			         {          glUniform1i( glGetUniformLocation( shaderID[0], name.cbegin() ), v  		 			); }
    [[maybe_unused]] void 	      set1u(std::string_view const& name, u32 v		 				) 			         {         glUniform1ui( glGetUniformLocation( shaderID[0], name.cbegin() ), v  		 			); }
    [[maybe_unused]] void 	      set2f(std::string_view const& name, std::array<f32, 2> const& v) 			         {          glUniform2f( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1]			 	); }
    [[maybe_unused]] void 	      set2i(std::string_view const& name, std::array<i32, 2> const& v) 			         {          glUniform2i( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1]			 	); }
    [[maybe_unused]] void 	      set2u(std::string_view const& name, std::array<u32, 2> const& v) 			         {         glUniform2ui( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1]			 	); }
    [[maybe_unused]] void 	      set3f(std::string_view const& name, std::array<f32, 3> const& v) 			         {          glUniform3f( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1], v[2]		); }
    [[maybe_unused]] void 	      set3i(std::string_view const& name, std::array<i32, 3> const& v) 			         {          glUniform3i( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1], v[2]		); }
    [[maybe_unused]] void 	      set3u(std::string_view const& name, std::array<u32, 3> const& v) 			         {         glUniform3ui( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1], v[2]		); }
    [[maybe_unused]] void 	      set4f(std::string_view const& name, std::array<f32, 4> const& v) 			         {          glUniform4f( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1], v[2], v[3]	); }
    [[maybe_unused]] void 	      set4i(std::string_view const& name, std::array<i32, 4> const& v) 			         {          glUniform4i( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1], v[2], v[3]	); }
    [[maybe_unused]] void 	      set4u(std::string_view const& name, std::array<u32, 4> const& v) 			         {         glUniform4ui( glGetUniformLocation( shaderID[0], name.cbegin() ), v[0], v[1], v[2], v[3]	); }
    [[maybe_unused]] void 		 set1fv(std::string_view const& name, i64 count, const f32* buffer)    			     {         glUniform1fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set2fv(std::string_view const& name, i64 count, const f32* buffer)    			     {         glUniform2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set3fv(std::string_view const& name, i64 count, const f32* buffer)    			     {         glUniform3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set4fv(std::string_view const& name, i64 count, const f32* buffer)    			     {         glUniform4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set1iv(std::string_view const& name, i64 count, const i32* buffer)    			     {         glUniform1iv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set2iv(std::string_view const& name, i64 count, const i32* buffer)    			     {         glUniform2iv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set3iv(std::string_view const& name, i64 count, const i32* buffer)    			     {         glUniform3iv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set4iv(std::string_view const& name, i64 count, const i32* buffer)    			     {         glUniform4iv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set1uv(std::string_view const& name, i64 count, const u32* buffer)    			     {        glUniform1uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set2uv(std::string_view const& name, i64 count, const u32* buffer)    			     {        glUniform2uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set3uv(std::string_view const& name, i64 count, const u32* buffer)    			     {        glUniform3uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void 		 set4uv(std::string_view const& name, i64 count, const u32* buffer)    			     {        glUniform4uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, buffer 			  ); }
    [[maybe_unused]] void   setMatrix2fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) {   glUniformMatrix2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void   setMatrix3fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) {   glUniformMatrix3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void   setMatrix4fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) {   glUniformMatrix4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void setMatrix2x3fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) { glUniformMatrix2x3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void setMatrix3x2fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) { glUniformMatrix3x2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void setMatrix2x4fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) { glUniformMatrix2x4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void setMatrix4x2fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) { glUniformMatrix4x2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void setMatrix3x4fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) { glUniformMatrix3x4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void setMatrix4x3fv(std::string_view const& name, i64 count, const f32* buffer, bool transpose) { glUniformMatrix4x3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), count, transpose, buffer ); }
    [[maybe_unused]] void 		  set1fv(std::string_view const& name, std::vector<f32> const& buffer)    			   {         glUniform1fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set2fv(std::string_view const& name, std::vector<f32> const& buffer)    			   {         glUniform2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set3fv(std::string_view const& name, std::vector<f32> const& buffer)    			   {         glUniform3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set4fv(std::string_view const& name, std::vector<f32> const& buffer)    			   {         glUniform4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set1iv(std::string_view const& name, std::vector<i32> const& buffer)    			   {         glUniform1iv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set2iv(std::string_view const& name, std::vector<i32> const& buffer)    			   {         glUniform2iv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set3iv(std::string_view const& name, std::vector<i32> const& buffer)    			   {         glUniform3iv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set4iv(std::string_view const& name, std::vector<i32> const& buffer)    			   {         glUniform4iv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set1uv(std::string_view const& name, std::vector<u32> const& buffer)    			   {        glUniform1uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set2uv(std::string_view const& name, std::vector<u32> const& buffer)    			   {        glUniform2uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set3uv(std::string_view const& name, std::vector<u32> const& buffer)    			   {        glUniform3uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void 		  set4uv(std::string_view const& name, std::vector<u32> const& buffer)    			   {        glUniform4uiv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(),            buffer.begin().base() ); }
    [[maybe_unused]] void   setMatrix2fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) {   glUniformMatrix2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void   setMatrix3fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) {   glUniformMatrix3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void   setMatrix4fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) {   glUniformMatrix4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void setMatrix2x3fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) { glUniformMatrix2x3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void setMatrix3x2fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) { glUniformMatrix3x2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void setMatrix2x4fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) { glUniformMatrix2x4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void setMatrix4x2fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) { glUniformMatrix4x2fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void setMatrix3x4fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) { glUniformMatrix3x4fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }
    [[maybe_unused]] void setMatrix4x3fv(std::string_view const& name, std::vector<f32> const& buffer, bool transpose) { glUniformMatrix4x3fv( glGetUniformLocation( shaderID[0], name.cbegin() ), buffer.size(), transpose, buffer.begin().base() ); }


};




ShaderMetadata loadShader(std::string_view const& path, u8 stype);
