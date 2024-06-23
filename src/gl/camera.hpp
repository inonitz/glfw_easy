#pragma once
#include "util/vec.hpp"


struct Camera2D
{
    const math::vec3f m_up = { 0.0f, 1.0f, 0.0f };
    math::vec3f m_pos, m_front;
    math::vec2f m_rotate;
    f32 m_fov;
    f32 k_scroll_factor = 0.7f;
    f32 k_rotate_factor = 0.1f;
    f32 k_dx = 0.05f;
    math::mat4f m_transform;


    Camera2D() : 
        m_pos{0.0f, 0.0f, -3.0f}, 
        m_front{0.0f, 0.0f, 0.0f}, 
        m_rotate{0.0f},
        m_fov{45.0f} {}


    void update(f32 dt, math::vec2f winSize);


    auto& getTransform() const {
        return m_transform;
    }
};