//{{{  includes
#include "glviz.h"
#include "utility.h"

#include "camera.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <SDL.h>

#include <iostream>
#include <cstdlib>
#include <functional>

using namespace std;
//}}}

namespace GLviz {
  namespace {
    int m_screen_width, m_screen_height;
    unsigned int m_timer_msec = 16;

    SDL_Window* m_sdl_window;
    SDL_GLContext m_gl_context;

    Camera* m_camera;

    function<void()> m_guiCallback;
    function<void()> m_displayCallback;
    function<void()> m_closeCallback;
    function<void (unsigned int)> m_timerCallback;
    function<void (SDL_Keycode)> m_keyboardCallback;
    function<void (int width, int height)> m_reshapeCallback;

    //{{{
    void reshape (int width, int height) {

      m_screen_width  = width;
      m_screen_height = height;

      if (m_reshapeCallback)
        m_reshapeCallback (width, height);
    }
    //}}}
    //{{{
    void mouse (int button, int state, int x, int y) {

      const float xf = static_cast<float>(x) / static_cast<float>(m_screen_width);
      const float yf = static_cast<float>(y) / static_cast<float>(m_screen_height);

      //cout << "mouse " << xf << "," << yf << " " << x << "," << y << " "
      //                 << m_screen_width << "," << m_screen_height << endl;

      m_camera->trackball_begin_motion (xf, yf);
      }
    //}}}
    //{{{
    void motion (int state, int x, int y) {

      const float xf = static_cast<float>(x) / static_cast<float>(m_screen_width);
      const float yf = static_cast<float>(y) / static_cast<float>(m_screen_height);

      //cout << "motion " << xf << "," << yf << " " << x << "," << y << " "
      //                  << m_screen_width << "," << m_screen_height << endl;

      if (state & SDL_BUTTON_LMASK)
        m_camera->trackball_end_motion_rotate (xf, yf);
      else if (state & SDL_BUTTON_RMASK)
        m_camera->trackball_end_motion_zoom (xf, yf);
      else if (state & SDL_BUTTON_MMASK)
        m_camera->trackball_end_motion_translate (xf, yf);
      }
    //}}}
    //{{{
    bool process_events() {

      bool quit = false;
      SDL_Event event;

      while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        ImGuiIO const& io = ImGui::GetIO();

        switch (event.type) {
          case SDL_KEYDOWN:
            if (!io.WantCaptureKeyboard && m_keyboardCallback)
              m_keyboardCallback(event.key.keysym.sym);
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
            if (!io.WantCaptureMouse) {
              SDL_CaptureMouse(SDL_TRUE);
              mouse(event.button.button, event.button.state, event.button.x, event.button.y);
              }
            break;

          case SDL_MOUSEMOTION:
            if (!io.WantCaptureMouse)
              motion(event.motion.state, event.motion.x, event.motion.y);
            break;

          case SDL_QUIT:
            quit = true;
            break;
          }
        }

      return quit;
      }
    //}}}
    }

  int screen_width() { return m_screen_width; }
  int screen_height() { return m_screen_height; }

  Camera* camera() { return m_camera; }
  void set_camera (Camera& camera) { m_camera = &camera; }

  //{{{
  void guiCallback (function<void()> guiCallback) { m_guiCallback = guiCallback; }
  //}}}
  //{{{
  void displayCallback (function<void ()> displayCallback) { m_displayCallback = displayCallback; }
  //}}}
  //{{{
  void closeCallback (function<void ()> closeCallback) { m_closeCallback = closeCallback; }
  //}}}
  //{{{
  void timerCallback (function<void (unsigned int)> timerCallback, unsigned int timer_msec)
  {
      m_timerCallback = timerCallback;
      m_timer_msec = timer_msec;
  }
  //}}}
  //{{{
  void keyboardCallback (function<void (SDL_Keycode)> keyboardCallback) {
      m_keyboardCallback = keyboardCallback;
  }
  //}}}
  //{{{
  void reshapeCallback (function<void (int width, int height)> reshapeCallback) {
    m_reshapeCallback = reshapeCallback;
    }
  //}}}

  //{{{
  void cout_opengl_version() {

    GLint context_major_version, context_minor_version, context_profile;

    glGetIntegerv (GL_MAJOR_VERSION, &context_major_version);
    glGetIntegerv (GL_MINOR_VERSION, &context_minor_version);
    glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &context_profile);

    cout << "  OpenGL version " << context_major_version << "." << context_minor_version << " ";

    switch (context_profile) {
      case GL_CONTEXT_CORE_PROFILE_BIT:
        cout << "core";
        break;

      case GL_CONTEXT_COMPATIBILITY_PROFILE_BIT:
        cout << "compatibility";
        break;
      }

    cout << " profile context" << endl;
    }
  //}}}
  //{{{
  void cout_glew_version() {
    cout << "  GLEW version " << glewGetString(GLEW_VERSION) << "." << endl;
    }
  //}}}

  //{{{
  int exec (Camera& camera) {

    m_camera = &camera;
    Uint32 last_time = 0;

    reshape (m_screen_width, m_screen_height);

    while (!process_events()) {
      if (m_timerCallback) {
        const Uint32 time = SDL_GetTicks();
        const Uint32 delta_t_msec = time - last_time;
        if (delta_t_msec >= m_timer_msec) {
         last_time = time;
          m_timerCallback (delta_t_msec);
          }
        }

      if (m_displayCallback)
        m_displayCallback();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame (m_sdl_window);
      ImGui::NewFrame();

      if (m_guiCallback)
        m_guiCallback();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());

      SDL_GL_SwapWindow (m_sdl_window);
      }

    if (m_closeCallback)
      m_closeCallback();

    SDL_GL_DeleteContext (m_gl_context);
    SDL_DestroyWindow (m_sdl_window);
    SDL_Quit();

    return EXIT_SUCCESS;
    }
  //}}}
  //{{{
  void GLviz (int screen_width, int screen_height) {

    m_screen_width = screen_width;
    m_screen_height = screen_height;

    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      //{{{  error,return
        // Initialize GLEW.
      cerr << "Failed to initialize SDL Video:" << endl;
      cerr << "Error: " << SDL_GetError() << endl;
      SDL_Quit();
      exit(EXIT_FAILURE);
      }
      //}}}

    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
    //SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
    //SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    //SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // no WSL
    cout << "multiSample disabled" << endl;
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    m_sdl_window = SDL_CreateWindow ("GLviz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                     m_screen_width, m_screen_height,
                                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_sdl_window) {
      //{{{  error, return
      cerr << "Failed to create SDL window:" << endl;
      cerr << "Error: " << SDL_GetError() << endl;

      SDL_Quit();
      exit (EXIT_FAILURE);
      }
      //}}}

    m_gl_context = SDL_GL_CreateContext(m_sdl_window);
    if (!m_gl_context) {
      //{{{  error, return
      cerr << "Failed to initialize OpenGL:" << endl;
      cerr << "Error: " << SDL_GetError() << endl;

      SDL_Quit();
      exit(EXIT_FAILURE);
      }
      //}}}

    // Print OpenGL version.
    cout_opengl_version();

      { // Initialize GLEW.
      glewExperimental = GL_TRUE;
      GLenum glew_error = glewInit();

      if (GLEW_OK != glew_error) {
        //{{{  erro, return
        cerr << "Failed to initialize GLEW:" << endl;
        cerr << __FILE__ << "(" << __LINE__ << "): " << glewGetErrorString(glew_error) << endl;
        exit(EXIT_FAILURE);
        }
        //}}}

      // GLEW has a problem with core contexts. It calls
      // glGetString(GL_EXTENSIONS), which causes a GL_INVALID_ENUM error.
      GLenum gl_error = glGetError();
      if (GL_NO_ERROR != gl_error && GL_INVALID_ENUM != gl_error)
        cerr << __FILE__ << "(" << __LINE__ << "): " << GLviz::get_gl_error_string(gl_error) << endl;
      }

    // print GLEW version.
    cout_glew_version();
    cout << endl;

    // Initialize ImGui.
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(m_sdl_window, m_gl_context);
    ImGui_ImplOpenGL3_Init();
    }
  //}}}
  }
