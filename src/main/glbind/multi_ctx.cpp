#include "test.hpp"
#include <iostream>
#include <glbinding/glbinding.h>
#include <glbinding/gl/gl.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/debug.h>
#include <GLFW/glfw3.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include "util/marker2.hpp"


using namespace gl;
using namespace glbinding;


struct PerWindow {
    unsigned int id;
};


void key_callback(GLFWwindow * window, int key, int /*scancode*/, int action, int /*mods*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }
}


void framebuffer_callback(GLFWwindow* window, int width, int height)
{
    // glfwMakeContextCurrent(window);
    PerWindow* __userdata = reinterpret_cast<PerWindow*>(glfwGetWindowUserPointer(window));
    glbinding::useContext(__userdata->id);
    gl::glViewport(0, 0, width, height);
    return;
}


int test_multi_context()
{
    glfwSetErrorCallback([](int errnum, const char* errmsg) {
        std::cerr << "GLFW ERROR " << errnum << ": " << errmsg << std::endl;
    });
    if (!glfwInit())
        return 1;
    IMGUI_CHECKVERSION();


    glfwDefaultWindowHints();
#ifdef SYSTEM_DARWIN
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    int activeWindow = 0;
    int glver = 0;
    PerWindow windowUserData[2] = {
        {0}, {1}
    };
    ImGuiContext* imgui_ctx[2] = {
        ImGui::CreateContext(),
        ImGui::CreateContext()
    };
    GLFWwindow * window[2] = {
        glfwCreateWindow(640, 480, "", nullptr, nullptr),
        glfwCreateWindow(640, 480, "", nullptr, nullptr)
    };
    if (!window[0] || !window[1])
    {
        glfwTerminate();
        return -1;
    }


    mark(); glfwMakeContextCurrent(window[0]);
    mark(); glfwSetWindowUserPointer      (window[0], &windowUserData[0]  );
    mark(); glfwSetKeyCallback            (window[0], key_callback        );
    mark(); glfwSetFramebufferSizeCallback(window[0], framebuffer_callback);
    mark(); glbinding::initialize(0, glfwGetProcAddress, false);
    mark(); glbinding::aux::enableGetErrorCallback();
    mark(); ImGui::SetCurrentContext(imgui_ctx[0]);
    mark(); ImGui::StyleColorsDark();
    mark(); glver = ImGui_ImplGlfw_InitForOpenGL(window[0], false);
    mark(); glver = glver && ImGui_ImplOpenGL3_Init("#version 460");
    if(!glver)
        return -1;


    mark(); glfwMakeContextCurrent(window[1]);
    mark(); glfwSetWindowUserPointer      (window[1], &windowUserData[1]  );
    mark(); glfwSetKeyCallback            (window[1], key_callback        );
    mark(); glfwSetFramebufferSizeCallback(window[1], framebuffer_callback);
    mark(); glbinding::initialize(1, glfwGetProcAddress, false);
    mark(); glbinding::aux::enableGetErrorCallback();
    mark(); ImGui::SetCurrentContext(imgui_ctx[1]);
    mark(); ImGui::StyleColorsDark();
    mark(); glver = ImGui_ImplGlfw_InitForOpenGL(window[1], false);
    mark(); glver = glver && ImGui_ImplOpenGL3_Init("#version 460");
    if(!glver)
        return -1;


    while (!glfwWindowShouldClose(window[0]) && !glfwWindowShouldClose(window[1]))
    {
        glfwPollEvents();
        if ((activeWindow % 2) == 0)
        {
            glfwMakeContextCurrent(window[0]);
            glbinding::useContext(0);
            ImGui::SetCurrentContext(imgui_ctx[0]);
            gl::glClearColor(1.0f, 0.0f, activeWindow / 15.0f, 0.0f);
            /* Begin */
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            /* Actual Rendering Code */
            gl::glClear(gl::ClearBufferMask{GL_COLOR_BUFFER_BIT});
            /* End */
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window[0]);
        }
        else if ((activeWindow % 2) == 1)
        {
            glfwMakeContextCurrent(window[1]);
            glbinding::useContext(1);
            ImGui::SetCurrentContext(imgui_ctx[1]);
            gl::glClearColor(0.0f, 1.0f, activeWindow / 15.0f, 0.0f);
            /* Begin */
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            /* Actual Rendering Code */
            gl::glClear(gl::ClearBufferMask{GL_COLOR_BUFFER_BIT});
            /* End */
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window[1]);
        }
        activeWindow = (activeWindow + 1) % 16;
    }

    ImGui::SetCurrentContext(imgui_ctx[0]);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    ImGui::SetCurrentContext(imgui_ctx[1]);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}