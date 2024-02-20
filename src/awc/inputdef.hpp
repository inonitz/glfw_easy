#ifndef __AWC_INPUT_DEFINITION_HEADER__
#define __AWC_INPUT_DEFINITION_HEADER__
#include "util/base.hpp"


namespace AWC::Input {


enum class keyCode : u8 {
  ESCAPE,
  SPACE,
  NUM0,
  NUM1,
  NUM2,
  NUM3,
  NUM4,
  NUM5,
  NUM6,
  NUM7,
  NUM8,
  NUM9,
  W,
  A,
  S,
  D,
  Q,
  E,
  R,
  F,
  C,
  X,
  Z,
  T,
  G,
  V,
  B,
  H,
  Y,
  U,
  J,
  N,
  M,
  K,
  I,
  O,
  L,
  P,
  KEY_MAX
};

enum class mouseButton : u8 { LEFT, RIGHT, MIDDLE, MAX };

enum class inputState : u8 {
  DEFAULT = 0, /* mapping is (1 << GLFW_KEY_STATE) */
  RELEASE = 1,
  PRESS   = 2,
  REPEAT  = 4,
  MAX     = 8
};


u16         toGLFWKeyCode(keyCode kc);
keyCode     toKeyCode(u16 glfw);
const char* keyCodeToString(keyCode kc);
u16         toGLFWMouseButton(mouseButton button);
mouseButton toMouseButton(u16 glfw);


} // namespace AWC::Input

#endif