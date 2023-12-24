#include "awc_internal.hpp"


namespace AWC::GL {

GladGLContext const* cinst() {
    return AWC::activeContext().opengl;
}


} // namespace AWC::GL
