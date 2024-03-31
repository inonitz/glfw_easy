#pragma once
#include "util/vec.hpp"



struct ProjectionParameters
{
    math::vec2f lr;
    math::vec2f tb;
    math::vec2f nf;
};


class OrthographicCamera
{
public:
    void create(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
    void destroy();
    void onUpdate(f32 dt, ProjectionParameters* optionalPerspectiveUpdate = nullptr);


    math::mat4f const& getTransform() { 
        math::MultiplyMat4Mat4(projection, view, pv);
        return pv;
    }

private:
    void recalculateView();
    void recalculateProjection();

private:
    math::mat4f pv;
    math::mat4f projection;
    math::mat4f view;

    ProjectionParameters pargs;
    
    math::vec2f m_pos;
    math::vec2f m_vel;
};