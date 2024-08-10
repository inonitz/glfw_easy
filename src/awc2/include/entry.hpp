#ifndef __AWC2_ENTRY_INTERFACE_HEADER__
#define __AWC2_ENTRY_INTERFACE_HEADER__


/* 
    [NOTE]: 
    File Rename required, 
    'entry' doesn't describe the purpose of this file fully
*/
namespace AWC2 {
    static constexpr unsigned int k_maximumContexts = 8;


    void init();
    void destroy();


    __attribute__((hot)) void newFrame();
} // namespace AWC2


#endif