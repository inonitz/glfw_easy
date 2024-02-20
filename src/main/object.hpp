#pragma once
#include "util/base.hpp"
#include <array>
#include <vector>
#include <optional>
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"


#define CPU_SIDE
#define GPU_SIDE


template<bool usingElementBuffer>
struct RenderData {
    using VAO = VertexArray<usingElementBuffer>; 
    /*
        Modify VAO to NOT use templates,
        but instead use optionals for the internal buffers
        ex. use a std optional for the Element Buffer 
    */
    std::vector<f32>      CPU_SIDE m_primData;
    Buffer                GPU_SIDE m_vertices;
    std::optional<Buffer> GPU_SIDE m_indices;
    VAO                   GPU_SIDE m_vertexDesc;
    TextureBuffer         GPU_SIDE m_texture;
    bool                           m_vertexAttribBits; /* Which attributes are enabled */

    math::mat4f m_model, m_projection;
};


struct GeometryData {
    /* 
        Lines, Points, Shapes, etc... that define the boundaries
        of the Geometry - basically position data but more refined.
    */
};


struct PhysicsData {
    /* 
        Acting Forces, Collision Detection & Impulses, 
        Constraint's & Solvers, etc...
    */
};


template<uint32_t ObjectCount> struct PrimitiveArray {
    std::array<RenderData  , ObjectCount> render;
    std::array<GeometryData, ObjectCount> geom;
    std::array<PhysicsData , ObjectCount> physics;
};