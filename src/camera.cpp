#include "camera.hpp"




void OrthographicCamera::create(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) 
{
    
    pargs.lr = { left, right  };
    pargs.tb = { top,  bottom };
    pargs.nf = { near, far    };
    m_pos = math::vec2f{ 0.0f };
    m_vel      = math::vec2f{ 0.0f };
    math::orthographic(pargs.lr, pargs.tb, pargs.nf, projection);
    return;
}


void OrthographicCamera::destroy() 
{
    pargs.lr.mem.zero();
    pargs.tb.mem.zero();
    pargs.nf.mem.zero();
    m_pos.mem.zero();
    m_vel.mem.zero();
    view.mem.zero();
    projection.mem.zero();

}


void OrthographicCamera::onUpdate(__unused f32 dt, ProjectionParameters* pparam)
{
    if(pparam != nullptr) {
        pargs = *pparam;
        recalculateProjection();
    }

    // math::vec2f d = getCursorDelta<f32>();
    math::vec2f d{0.0f};
    if( (d.x + d.y) < 1e-8f ) {
        m_pos += m_vel * d;
    }
}


void OrthographicCamera::recalculateView()
{
    
}


void OrthographicCamera::recalculateProjection()
{
    math::orthographic(pargs.lr, pargs.tb, pargs.nf, projection);
    return;
}


