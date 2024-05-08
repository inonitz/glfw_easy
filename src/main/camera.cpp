#include "camera.hpp"
#include "awc/awc.hpp"
#include "awc/inputdef.hpp"
#include "util/vec.hpp"



void Camera2D::update(__unused f32 dt)
{
    math::vec2f scrollDelta, mouseDelta, prevMousePos, currMousePos;


    scrollDelta = AWC::Input::getMouseScrollDelta();
    prevMousePos = AWC::Input::getPreviousMousePosition();
    currMousePos = AWC::Input::getMousePosition();
    mouseDelta = AWC::Input::getMousePositionDelta();
    if(AWC::Input::isMouseScrollMoving()) {
        printf("Scroll %f\n", scrollDelta.y);
        m_scale *= (scrollDelta.y > 0.0f) ? k_scroll_factor : (1.0f / k_scroll_factor);
        m_scale.print();
    }
    if(AWC::Input::isMouseButtonRepeated(AWC::Input::mouseButton::LEFT)
        && AWC::Input::isMouseMoving()
    ) {
        printf("Mouse\n");
        m_rotate += math::dot(currMousePos, prevMousePos) / (prevMousePos.length() * currMousePos.length());
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