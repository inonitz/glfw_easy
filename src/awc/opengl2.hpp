#ifndef __AWC_OPENGL_DEFINITION_HEADER__
#define __AWC_OPENGL_DEFINITION_HEADER__
#ifndef AWC_OPENGL_DEFINE_IMPLEMENTATION
    #define AWC_OPENGL_DEFINE_IMPLEMENTATION 0
#endif
#ifndef AWC_OPENGL_LOG_COMMANDS
    #define AWC_OPENGL_LOG_COMMANDS 0
#endif


namespace AWC {


void opengl_global_create();
void opengl_global_destroy();


namespace Context::OpenGL {
// #if defined(_DEBUG) || AWC_OPENGL_DEFINE_IMPLEMENTATION
    void create(unsigned char context_id);
    void destroy(unsigned char context_id);
// #endif
} // namespace Context::OpenGL


} // namespace AWC


#endif