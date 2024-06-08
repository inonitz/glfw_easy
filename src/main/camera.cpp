#include "camera.hpp"
#include "awc/awc.hpp"
#include "awc/inputdef.hpp"
#include "util/vec.hpp"
#include <algorithm>
// #include "util/marker.hpp"


// void Camera2D::update(__unused f32 dt, math::vec2f winSize)
// {
//     const i8 keyMap[6] = {
//         (AWC::Input::isKeyPressed(AWC::Input::keyCode::A) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::A)),
//         (AWC::Input::isKeyPressed(AWC::Input::keyCode::D) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::D)),
//         (AWC::Input::isKeyPressed(AWC::Input::keyCode::S) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::S)),
//         (AWC::Input::isKeyPressed(AWC::Input::keyCode::W) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::W)),
//         (AWC::Input::isKeyPressed(AWC::Input::keyCode::G) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::H)),
//         (AWC::Input::isKeyPressed(AWC::Input::keyCode::H) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::H))
//     };


//     if(AWC::Input::isMouseMoving() && AWC::Input::isMouseButtonPressed(AWC::Input::mouseButton::RIGHT)) {
//         math::vec3f direction;
//         math::vec2f tmp = AWC::Input::getPreviousMousePosition();
//         math::vec2f mouseDelta = AWC::Input::getMousePosition();

//         mouseDelta -= tmp;
//         m_rotate += k_rotate_factor * mouseDelta;
//         m_rotate.pitch = std::clamp(m_rotate.pitch, -89.0f, 89.0f);
        
//         tmp = { math::radians(m_rotate.yaw), math::radians(m_rotate.pitch) };
//         direction = {
//             cosf(tmp.x) * cosf(tmp.y),
//             sinf(tmp.y),
//             sinf(tmp.x) * cosf(tmp.y)
//         };
//         m_front = direction.normalized();


//         auto* front_str  = m_front.to_string();
//         auto* rotate_str = m_rotate.to_string();
//         markfmt("\n  =>  (front):  %s\n  => (rotate):  %s\n", front_str, rotate_str);
//         free(front_str);
//         free(rotate_str);
//     }
//     m_fov += AWC::Input::isMouseScrollMoving() * AWC::Input::getMouseScrollOffset()[1];
//     m_fov = std::clamp(m_fov, 1.0f, 45.0f);
//     if( AWC::Input::isMouseScrollMoving() )
//         markfmt(" ( fov ): %2.2f", m_fov); 


//     if( keyMap[3] ) {
//         m_pos += k_dx * dt * m_front;
//     }
//     if( keyMap[2] ) {
//         m_pos -= k_dx * dt * m_front;
//     }
//     if( keyMap[1] ) {
//         m_pos += k_dx * dt * math::vec3f{math::cross(m_front, m_up)}.normalized();
//     }
//     if( keyMap[0] ) {
//         m_pos 
//         -= k_dx * dt * math::vec3f{math::cross(m_front, m_up)}.normalized();
//     }
//     if(keyMap[3] || keyMap[2] || keyMap[1] || keyMap[0]) {
//         auto* pos_str = m_pos.to_string();   
//         markfmt(" (pos): %s", pos_str);
//         free(pos_str);
//     }


//     math::mat4f T, R, S, TRS, PVM, P, V, tmp;
//     math::lookAt(m_pos, m_pos + m_front, m_up, V);
//     math::perspective(winSize.x / winSize.y, 60.0f, 0.01f, 100.0f, P);

//     math::MultiplyMat4Mat4(P, V, m_transform);
// 	// math::rotate({ 0.0f, 1.0f, 0.0f }, m_rotate * k_rotate_factor, R);
//     // math::scale(m_scale, S);
// 	// math::MultiplyMat4Mat4(R, S, TRS);

//     // math::MultiplyMat4Mat4(P,   V,   tmp);
//     // math::MultiplyMat4Mat4(tmp, TRS, PVM);
//     // m_transform = tmp;
//     return;
// }


void Camera2D::update(__unused f32 dt, math::vec2f winSize)
{
    static f32 rot_theta;
    static math::vec3f translate{0.0f};
    static math::vec3f scale{1.0f};


    if(AWC::Input::isMouseScrollMoving()) {
        math::vec2f scrollDelta = AWC::Input::getMouseScrollOffset();
        scale *= (scrollDelta.y > 0.0f) ? k_scroll_factor : (1.0f / k_scroll_factor);
    }
    if(AWC::Input::isMouseMoving()) {
        math::vec2f prevMousePos = AWC::Input::getPreviousMousePosition();
        math::vec2f currMousePos = AWC::Input::getMousePosition();
        f32 alpha;
        prevMousePos *= math::vec2f{1.0f} / winSize;
        currMousePos *= math::vec2f{1.0f} / winSize;
        prevMousePos.y = 1.0f - prevMousePos.y;
        currMousePos.y = 1.0f - currMousePos.y;
        prevMousePos = math::vec2f{-1.0f} + 2.0f * prevMousePos;
        currMousePos = math::vec2f{-1.0f} + 2.0f * currMousePos;
        
        if( AWC::Input::isMouseButtonPressed(AWC::Input::mouseButton::LEFT) ) {
            alpha = ( prevMousePos.length() * currMousePos.length() );
            alpha = std::clamp( math::dot(prevMousePos, currMousePos) / alpha, -1.0f, 1.0f );
            alpha = std::acos(alpha);
            rot_theta += math::degrees(alpha);
        }
        else if( AWC::Input::isMouseButtonPressed(AWC::Input::mouseButton::RIGHT) ) {
            currMousePos -= prevMousePos;
            currMousePos /= dt;
            translate += { currMousePos.x, currMousePos.y, 0.0f };
        }
    }


    const i8 keyMap[6] = {
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::A) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::A)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::D) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::D)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::S) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::S)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::W) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::W)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::G) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::H)),
        (AWC::Input::isKeyPressed(AWC::Input::keyCode::H) || AWC::Input::isKeyRepeated(AWC::Input::keyCode::H))
    };

    translate.x += (keyMap[1] - keyMap[0]) * k_dx;
    translate.y += (keyMap[3] - keyMap[2]) * k_dx;
    math::mat4f T, R, S;
    math::translate(translate, T); 
    /* ^^^ 
        Look At Last Tabs Opened - matrix should be transposed. 
        This is a problem either with math::translate() / math::MultiplyMat4Mat4() ... 
        Find & Fix Please :))) (took me a week to find this I wanna die)
    */
	math::rotate(m_up, rot_theta, R);
	math::scale(scale, S);
    m_transform = T;
    return;
}