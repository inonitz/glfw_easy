#ifndef __AWC2_CONTEXT_INTERFACE_HEADER__
#define __AWC2_CONTEXT_INTERFACE_HEADER__


namespace AWC2 {
    using ContextID = unsigned char;
    
    struct ViewportSize {
        unsigned int x;
        unsigned int y;
    };


    ContextID createContext();
    void      destroyContext(ContextID id);
    
    void initializeContext(ContextID id);
    void setCurrentContext(ContextID id = 0);
    bool isContextActive(ContextID id);
    void closeCurrentContext();
} // namespace AWC2::Context


#endif