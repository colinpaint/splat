// This file is part of GLviz.
//
// Copyright(c) 2014-2018 Sebastian Lipponer
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "glviz.hpp"
#include "utility.hpp"

#include "camera.hpp"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <iostream>
#include <cstdlib>
#include <functional>

namespace GLviz
{

namespace
{

int m_screen_width, m_screen_height;
unsigned int m_timer_msec = 16;

SDL_Window* m_sdl_window;
SDL_GLContext m_gl_context;

std::function<void ()>                      m_display_callback;
std::function<void ()>                      m_gui_callback;
std::function<void (unsigned int)>          m_timer_callback;
std::function<void (int width, int height)> m_reshape_callback;
std::function<void (SDL_Keycode)>           m_keyboard_callback;
std::function<void ()>                      m_close_callback;

Camera*  m_camera;

void
reshape(int width, int height)
{
    m_screen_width  = width;
    m_screen_height = height;

    if (m_reshape_callback)
    {
        m_reshape_callback(width, height);
    }
}

void
mouse(int button, int state, int x, int y)
{
    const float xf = static_cast<float>(x)
        / static_cast<float>(m_screen_width);
    const float yf = static_cast<float>(y)
        / static_cast<float>(m_screen_height);

    switch (button)
    {
        case SDL_BUTTON_LEFT:
            m_camera->trackball_begin_motion(xf, yf);
            break;

        case SDL_BUTTON_RIGHT:
            m_camera->trackball_begin_motion(xf, yf);
            break;

        case SDL_BUTTON_MIDDLE:
            m_camera->trackball_begin_motion(xf, yf);
            break;
    }
}

void
motion(int state, int x, int y)
{
    const float xf = static_cast<float>(x)
        / static_cast<float>(m_screen_width);
    const float yf = static_cast<float>(y)
        / static_cast<float>(m_screen_height);

    if (state & SDL_BUTTON_LMASK)
    {
        m_camera->trackball_end_motion_rotate(xf, yf);
    }
    else if (state & SDL_BUTTON_RMASK)
    {
        m_camera->trackball_end_motion_zoom(xf, yf);
    }
    else if (state & SDL_BUTTON_MMASK)
    {
        m_camera->trackball_end_motion_translate(xf, yf);
    }
}

bool
process_events()
{
    bool quit = false;
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        ImGuiIO const& io = ImGui::GetIO();

        switch (event.type)
        {
            case SDL_KEYDOWN:
                if (!io.WantCaptureKeyboard && m_keyboard_callback)
                    m_keyboard_callback(event.key.keysym.sym);
                break;
            case SDL_KEYUP:
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED)
                    reshape(event.window.data1, event.window.data2);
                break;
            case SDL_MOUSEBUTTONUP:
                if (!io.WantCaptureMouse)
                    SDL_CaptureMouse(SDL_FALSE);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (!io.WantCaptureMouse)
                {
                    SDL_CaptureMouse(SDL_TRUE);
                    mouse(event.button.button, event.button.state,
                        event.button.x, event.button.y);
                }
                break;
            case SDL_MOUSEMOTION:
                if (!io.WantCaptureMouse)
                    motion(event.motion.state, event.motion.x,
                        event.motion.y);
                break;
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    return quit;
}

}

int
screen_width()
{
    return m_screen_width;
}

int
screen_height()
{
    return m_screen_height;
}

Camera*
camera()
{
    return m_camera;
}

void
set_camera(Camera& camera)
{
    m_camera = &camera;
}

void
display_callback(std::function<void ()> display_callback)
{
    m_display_callback = display_callback;
}

void
reshape_callback(std::function<void (int width, int height)> reshape_callback)
{
    m_reshape_callback = reshape_callback;
}

void
timer_callback(std::function<void (unsigned int)> timer_callback,
    unsigned int timer_msec)
{
    m_timer_callback = timer_callback;
    m_timer_msec = timer_msec;
}

void
close_callback(std::function<void ()> close_callback)
{
    m_close_callback = close_callback;
}

void
gui_callback(std::function<void()> gui_callback)
{
    m_gui_callback = gui_callback;
}

void
keyboard_callback(std::function<void (SDL_Keycode)> keyboard_callback)
{
    m_keyboard_callback = keyboard_callback;
}

void
cout_opengl_version()
{
    GLint context_major_version, context_minor_version, context_profile;

    glGetIntegerv(GL_MAJOR_VERSION, &context_major_version);
    glGetIntegerv(GL_MINOR_VERSION, &context_minor_version);
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &context_profile);

    std::cout << "  OpenGL version " << context_major_version << "."
        << context_minor_version << " ";

    switch (context_profile)
    {
        case GL_CONTEXT_CORE_PROFILE_BIT:
            std::cout << "core";
            break;

        case GL_CONTEXT_COMPATIBILITY_PROFILE_BIT:
            std::cout << "compatibility";
            break;
    }

    std::cout << " profile context." << std::endl;
}

void
cout_glew_version()
{
    std::cout << "  GLEW version " << glewGetString(GLEW_VERSION)
        << "." << std::endl;
}

void
GLviz(int screen_width, int screen_height)
{
    m_screen_width = screen_width;
    m_screen_height = screen_height;

    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialize SDL Video:" << std::endl;
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
        SDL_GL_CONTEXT_PROFILE_CORE);

    //SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
    //SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
    //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    m_sdl_window = SDL_CreateWindow("GLviz",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        m_screen_width, m_screen_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!m_sdl_window)
    {
        std::cerr << "Failed to create SDL window:" << std::endl;
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    m_gl_context = SDL_GL_CreateContext(m_sdl_window);
    if (!m_gl_context)
    {
        std::cerr << "Failed to initialize OpenGL:" << std::endl;
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        std::exit(EXIT_FAILURE);
    }

    // Print OpenGL version.
    cout_opengl_version();

    // Initialize GLEW.
    {
        glewExperimental = GL_TRUE;
        GLenum glew_error = glewInit();

        if (GLEW_OK != glew_error)
        {
            std::cerr << "Failed to initialize GLEW:" << std::endl;
            std::cerr << __FILE__ << "(" << __LINE__ << "): "
                      << glewGetErrorString(glew_error) << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // GLEW has a problem with core contexts. It calls
        // glGetString(GL_EXTENSIONS), which causes a GL_INVALID_ENUM error.
        GLenum gl_error = glGetError();
        if (GL_NO_ERROR != gl_error && GL_INVALID_ENUM != gl_error)
        {
            std::cerr << __FILE__ << "(" << __LINE__ << "): "
                      << GLviz::get_gl_error_string(gl_error) << std::endl;
        }
    }

    // Print GLEW version.
    cout_glew_version();
    std::cout << std::endl;

    // Initialize ImGui.
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(m_sdl_window, m_gl_context);
    ImGui_ImplOpenGL3_Init();
}

int
exec(Camera& camera)
{
    m_camera = &camera;
    Uint32 last_time = 0;

    reshape(m_screen_width, m_screen_height);

    while (!process_events())
    {
        if (m_timer_callback)
        {
            const Uint32 time = SDL_GetTicks();
            const Uint32 delta_t_msec = time - last_time;

            if (delta_t_msec >= m_timer_msec)
            {
                last_time = time;
                m_timer_callback(delta_t_msec);
            }
        }

        if (m_display_callback) { m_display_callback(); }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(m_sdl_window);
        ImGui::NewFrame();

        if (m_gui_callback) { m_gui_callback(); }
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(m_sdl_window);
    }

    if (m_close_callback) { m_close_callback(); }

    SDL_GL_DeleteContext(m_gl_context);
    SDL_DestroyWindow(m_sdl_window);
    SDL_Quit();

    return EXIT_SUCCESS;
}

}
