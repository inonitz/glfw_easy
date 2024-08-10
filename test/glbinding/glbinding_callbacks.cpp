#include "test.hpp"
#include <iostream>
#include <glbinding/glbinding.h>
#include <glbinding/AbstractFunction.h>
#include <glbinding/Version.h>
#include <glbinding/CallbackMask.h>
#include <glbinding/FunctionCall.h>

#include <glbinding/gl32core/gl.h>

#include <glbinding-aux/Meta.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/ValidVersions.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding-aux/logging.h>
#include <GLFW/glfw3.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>


using namespace glbinding;


void error(int errnum, const char * errmsg)
{
    std::cerr << errnum << ": " << errmsg << std::endl;
}



struct vec2
{
    float x;
    float y;
};

const vec2 vertices[4] = { { +1.f, -1.f }, { +1.f, +1.f }, { -1.f, -1.f }, { -1.f, +1.f } };

const gl32core::GLchar * vert = R"(
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec2 a_vertex;

out vec4 color;

void main()
{
    gl32core::gl_Position = vec4(a_vertex, 0.0, 1.0);
    color = vec4(a_vertex * 0.5 + 0.5, 0.0, 1.0);
}
)";

const gl32core::GLchar * frag = R"(
#version 150
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) out vec4 fragColor;

in vec4 color;

void main()
{
    fragColor = color;
}
)";

gl32core::GLuint vao;
gl32core::GLuint quad;
gl32core::GLuint program;
gl32core::GLuint vs;
gl32core::GLuint fs;
gl32core::GLuint a_vertex;


void doGLStuff()
{
    gl32core::glViewport(0, 0, 320, 240);
    gl32core::glClear(gl32core::ClearBufferMask{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT});

    gl32core::glGenVertexArrays(1, &vao);
    gl32core::glGenBuffers(1, &quad);

    program = gl32core::glCreateProgram();
    vs = gl32core::glCreateShader(gl32core::GL_VERTEX_SHADER);
    gl32core::glShaderSource(vs, 1, &vert, 0);
    gl32core::glCompileShader(vs);
    fs = gl32core::glCreateShader(gl32core::GL_FRAGMENT_SHADER);
    gl32core::glShaderSource(fs, 1, &frag, 0);
    gl32core::glCompileShader(fs);
    gl32core::glAttachShader(program, vs);
    gl32core::glAttachShader(program, fs);
    gl32core::glLinkProgram(program);


    gl32core::glBindBuffer(gl32core::GL_ARRAY_BUFFER, quad);
    gl32core::glBufferData(gl32core::GL_ARRAY_BUFFER, sizeof(vec2) * 4, vertices, gl32core::GL_STATIC_DRAW);
    gl32core::glBindVertexArray(vao);

    a_vertex = static_cast<GLuint>(gl32core::glGetAttribLocation(program, "a_vertex"));
    gl32core::glEnableVertexAttribArray(static_cast<GLuint>(a_vertex));
    gl32core::glVertexAttribPointer(static_cast<GLuint>(a_vertex), 2, gl32core::GLenum{GL_FLOAT}, GL_FALSE, 0, nullptr);


    gl32core::glUseProgram(program);
    gl32core::glDrawArrays(gl32core::GLenum{GL_TRIANGLE_STRIP}, 0, 4);

    gl32core::glDeleteProgram(program);
    gl32core::glDeleteBuffers(1, &quad);
    gl32core::glDeleteVertexArrays(1, &vao);
    return;
}


static const auto after_callback_lambda = [](const glbinding::FunctionCall & call) 
{
    std::cout << call.function->name() << "(";

    for (unsigned i = 0; i < call.parameters.size(); ++i)
    {
        std::cout << call.parameters[i].get();
        if (i < call.parameters.size() - 1)
            std::cout << ", ";
    }

    std::cout << ")";

    if (call.returnValue)
    {
        std::cout << " -> " << call.returnValue.get();
    }

    std::cout << std::endl;
};



int test_glbinding()
{
    glbinding::addContextSwitchCallback([](ContextHandle handle){
        std::cout << "Activating context " << handle << std::endl;
    });


    glfwSetErrorCallback(error);
    if (!glfwInit())
        return 1;


    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_CLIENT_API,            GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE,       true);
    glfwWindowHint(GLFW_FOCUSED,       true);
    glfwWindowHint(GLFW_CENTER_CURSOR, true);
    glfwWindowHint(GLFW_RESIZABLE,     true);
    glfwWindowHint(GLFW_DECORATED,     true);
    glfwWindowHint(GLFW_REFRESH_RATE,  60);
    glfwWindowHint(GLFW_STENCIL_BITS, 24);
    glfwWindowHint(GLFW_DEPTH_BITS,   8);
    glfwWindowHint(GLFW_RED_BITS,     8);
    glfwWindowHint(GLFW_GREEN_BITS,   8);
    glfwWindowHint(GLFW_BLUE_BITS,    8);
    glfwWindowHint(GLFW_ALPHA_BITS,   8);
    GLFWwindow * window = glfwCreateWindow(320, 240, "", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);


    glbinding::initialize(0, glfwGetProcAddress, true, false);
    glbinding::useContext(0);
    glbinding::setCallbackMask(CallbackMask::Before | CallbackMask::ParametersAndReturnValue);
    glbinding::setBeforeCallback(after_callback_lambda);
    glbinding::aux::start();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 460");


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        doGLStuff();
        ImGui::Render();
        ImGui::EndFrame();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    std::cout << std::endl;

    glfwTerminate();
    return 0;
}
