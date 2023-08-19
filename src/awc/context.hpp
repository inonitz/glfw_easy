#ifndef __AWC_CONTEXT_HEADER__
#define __AWC_CONTEXT_HEADER__
#include "util/base.hpp"


namespace AWC {


void init();
void destroy();
void begin_frame();
void end_frame();

}  // namespace AWC


#endif