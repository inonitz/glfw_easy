#ifndef __AWC_WINDOW_HEADER__
#define __AWC_WINDOW_HEADER__
#include "windowdef.hpp"


namespace AWC {


namespace Event {
    typedef struct callbackTable callbackTable;
}

struct WindowContext
{
public:
    bool create(
        u32 width, 
        u32 height, 
        u64 windowOptions = 0
    );
    bool create(
        WindowDescriptor const& props,
        u64 windowOptions = 0
    );
    void destroy();
    void setCurrent() const;
    void setVerticalSync(u8 val) const;
    void setEventHooks(const Event::callbackTable* const) const;
    void close() const;
    bool shouldClose();

    u32  getWidth()  const { return m_data.desc.x; }
    u32  getHeight() const { return m_data.desc.y; }
    u32* getSize()      { return m_data.desc.dims; }
    GLFWwindow* underlying_handle() const { return m_data.desc.winHdl; }


    typedef struct alignpk(32) __packed_class_attributes {
        WindowDescriptor desc;
        WindowOptions    flags;
    } pod_data;


    auto& data() { return m_data; }
private:
    bool common_create(WindowOptions options);


    pod_data m_data;
};


} // namespace AWC

#endif