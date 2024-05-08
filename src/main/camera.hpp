#pragma once
#include "util/vec.hpp"



struct Camera2D
{
    math::vec2f m_translate, m_scale;
    f32 m_rotate;
    math::mat4f m_transform;
    f32 k_scroll_factor = 1.5f;
    f32 k_dx = 1.0f;


    Camera2D() : m_translate{0.0f}, m_scale(1.0, 1.0f), m_rotate(0.0f) {}


    void update(f32 dt);


    auto& getTransform() const {
        return m_transform;
    }
};