#include "camera.hpp"
#include "awc/awc.hpp"
#include "awc/inputdef.hpp"
#include "util/vec.hpp"



void Camera2D::update(__unused f32 dt, math::vec2f winSize)
{
    if(AWC::Input::isMouseScrollMoving()) {
        math::vec2f scrollDelta = AWC::Input::getMouseScrollOffset();
        m_scale *= (scrollDelta.y > 0.0f) ? k_scroll_factor : (1.0f / k_scroll_factor);
    }
    if(AWC::Input::isMouseButtonPressed(AWC::Input::mouseButton::LEFT)
        && AWC::Input::isMouseMoving()
    ) {
        math::vec2f prevMousePos = AWC::Input::getPreviousMousePosition();
        math::vec2f currMousePos = AWC::Input::getMousePosition();
        f32 alpha;
        prevMousePos *= math::vec2f{1.0f} / winSize;
        currMousePos *= math::vec2f{1.0f} / winSize;
        prevMousePos.y = 1.0f - prevMousePos.y;
        currMousePos.y = 1.0f - currMousePos.y;
        prevMousePos = math::vec2f{-1.0f} + 2.0f * prevMousePos;
        currMousePos = math::vec2f{-1.0f} + 2.0f * currMousePos;
        alpha = ( prevMousePos.length() * currMousePos.length() );
        alpha = std::clamp( math::dot(prevMousePos, currMousePos) / alpha, -1.0f, 1.0f );
        alpha = std::acos(alpha);
        m_rotate += math::degrees(alpha);


        m_translate = math::vec2f(currMousePos - prevMousePos) * -k_dx;
    }


    u8 keyMap[4] = {
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::A) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::A)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::D) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::D)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::S) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::S)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::W) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::W))
    };
    m_translate.x += (keyMap[1] - keyMap[0]) * k_dx;
    m_translate.y += (keyMap[3] - keyMap[2]) * k_dx;
    math::modelMatrix2d(m_translate, m_scale, m_rotate, m_transform);
    return;
}