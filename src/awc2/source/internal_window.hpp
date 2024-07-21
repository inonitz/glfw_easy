#ifndef __AWC2_INTERNAL_WINDOW_DEFINITION_HEADER__
#define __AWC2_INTERNAL_WINDOW_DEFINITION_HEADER__
#include "awc2/include/window_types.hpp"


typedef struct GLFWwindow GLFWwindow;


namespace AWC2::internal {


struct Window 
{
public:
    bool create(
        u16 width, 
        u16 height, 
        u64 windowOptions = 0,
        GLFWwindow* multi_window_opengl_context_share = nullptr
    );
    bool create(
        WindowDescriptor const& props,
        GLFWwindow* multi_window_opengl_context_share = nullptr
    );
    void destroy();
    void setCurrent() const;
    void swapBuffers() const;
    void setVerticalSync(u8 val) const;
    void close() const;
    bool shouldClose() const;
    bool isMinimized() const {
        return (m_data.description.stateFlags & WindowStateFlag::MINIMIZED)
            == WindowStateFlag::MINIMIZED; 
    }
    bool sizeChanged() const {
        return (m_data.description.stateFlags & WindowStateFlag::SIZE_CHANGED)
            == WindowStateFlag::SIZE_CHANGED; 
    }
    bool isFocused() const {
        return (m_data.description.stateFlags & WindowStateFlag::FOCUSED)
            == WindowStateFlag::FOCUSED; 
    }
    u32  getWidth()  const { return m_data.width; }
    u32  getHeight() const { return m_data.height; }
    GLFWwindow* underlying_handle()        const { return m_data.handle;        }
    GLFWwindow* underlying_parent_handle() const { return m_data.parent_handle; }

public:
    struct alignsz(8) WindowData {
        WindowDescriptor description;
        GLFWwindow* handle;
        GLFWwindow* parent_handle = nullptr;
        u16 width, height;
    };


    WindowData m_data;

private:
    bool common_create(WindowCreationFlag options, GLFWwindow* shared_win = nullptr);

};


} // namespace AWC2


#endif