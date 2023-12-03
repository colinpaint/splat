//{{{  includes
#include <cstdlib>
#include <functional>

#include "cApp.h"
#include "cCamera.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <SDL.h>

#include "../common/date.h"
#include "../common/cLog.h"

using namespace std;
//}}}

namespace {
  SDL_Window* mSdlWindow;
  SDL_GLContext mGlContext;
  }

//{{{
void cApp::init (int screenWidth, int screenHeight, bool fullScreen, bool multiSample) {

  mScreenWidth = screenWidth;
  mScreenHeight = screenHeight;
  mFullScreen = fullScreen;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    //{{{  error,return
      // Initialize GLEW.
    cLog::log (LOGERROR, fmt::format ("Failed to initialize SDL Video {}", SDL_GetError()));
    SDL_Quit();
    exit (EXIT_FAILURE);
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

  if (multiSample) {
    //{{{  multiSample
    SDL_GL_SetAttribute (SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute (SDL_GL_MULTISAMPLESAMPLES, 4);
    }
    //}}}
  mSdlWindow = SDL_CreateWindow ("app", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                 mScreenWidth, mScreenHeight,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  if (!mSdlWindow) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("Failed to create SDL window: {}", SDL_GetError()));
    SDL_Quit();
    exit (EXIT_FAILURE);
    }
    //}}}

  mGlContext = SDL_GL_CreateContext (mSdlWindow);
  if (!mGlContext) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("Failed to create initialize OpenGL: {}", SDL_GetError()));
    SDL_Quit();
    exit (EXIT_FAILURE);
    }
    //}}}
  //{{{  report openGL
  GLint context_major_version, context_minor_version, context_profile;
  glGetIntegerv (GL_MAJOR_VERSION, &context_major_version);
  glGetIntegerv (GL_MINOR_VERSION, &context_minor_version);
  glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &context_profile);
  cLog::log (LOGINFO, fmt::format ("openGL {}.{} {}",
                                   context_major_version, context_minor_version,
                                   (context_profile == GL_CONTEXT_CORE_PROFILE_BIT) ? "core" :
                                     (context_profile == GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) ?
                                       "compatibility" : "unknown"));
  //}}}

  //{{{  init, report glew
    {
    glewExperimental = GL_TRUE;
    GLenum glew_error = glewInit();
    if (glew_error != GLEW_OK) {
      //{{{  error, return
      cLog::log (LOGERROR, fmt::format ("Failed to create initialize GLEW {}",
                                        (const char*)(glewGetErrorString(glew_error))));
      exit(EXIT_FAILURE);
      }
      //}}}

    // GLEW has a problem with core contexts. It calls
    // glGetString(GL_EXTENSIONS), which causes a GL_INVALID_ENUM error.
    GLenum gl_error = glGetError();
    if (GL_NO_ERROR != gl_error && GL_INVALID_ENUM != gl_error)
      cLog::log (LOGERROR, fmt::format ("Gl {}", getGlErrorString (gl_error)));
    }
  cLog::log (LOGINFO, fmt::format ("glew {}", (const char*)glewGetString(GLEW_VERSION)));
  //}}}

  //{{{  init, report imGui.
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL (mSdlWindow, mGlContext);
  ImGui_ImplOpenGL3_Init();
  cLog::log (LOGINFO, fmt::format ("imGui {}", ImGui::GetVersion()));
  //}}}
  }
//}}}

//{{{
string cApp::getGlErrorString (GLenum glError) {

  switch (glError) {
    case GL_NO_ERROR:
      return "GL_NO_ERROR";

    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM";

    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE";

    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION";

    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";

    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY";

    default:
      return "UNKNOWN";
    }
  }
//}}}
//{{{
string cApp::getGlFramebufferStatusString (GLenum framebufferStatus) {

  switch (framebufferStatus) {
    case GL_FRAMEBUFFER_COMPLETE:
      return "GL_FRAMEBUFFER_COMPLETE";

    case GL_FRAMEBUFFER_UNDEFINED:
      return "GL_FRAMEBUFFER_UNDEFINED";

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";

    case GL_FRAMEBUFFER_UNSUPPORTED:
      return "GL_FRAMEBUFFER_UNSUPPORTED";

    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";

   case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";

    default:
      return "UNKNOWN";
    }
  }
//}}}

//{{{
void cApp::toggleFullScreen() {

  mFullScreen = !mFullScreen;
  if (mFullScreen)
    SDL_SetWindowFullscreen (mSdlWindow, SDL_WINDOW_FULLSCREEN);
  else
    SDL_SetWindowFullscreen (mSdlWindow, 0);
  }
//}}}

//{{{
int cApp::mainUILoop() {

  SDL_GL_SetSwapInterval (1);

  Uint32 last_time = 0;
  while (!processEvents()) {
    if (mTimerCallback) {
      const Uint32 time = SDL_GetTicks();
      const Uint32 deltaMs = time - last_time;
      if (deltaMs >= mTimerMs) {
        last_time = time;
        mTimerCallback();
        }
      }

    if (mDisplayCallback)
      mDisplayCallback();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame (mSdlWindow);
    ImGui::NewFrame();

    if (mGuiCallback)
      mGuiCallback();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData());

    SDL_GL_SwapWindow (mSdlWindow);
    }

  if (mCloseCallback)
    mCloseCallback();

  SDL_GL_DeleteContext (mGlContext);
  SDL_DestroyWindow (mSdlWindow);
  SDL_Quit();

  return EXIT_SUCCESS;
  }
//}}}

// private
//{{{
void cApp::resize (int width, int height) {

  mScreenWidth  = width;
  mScreenHeight = height;
  if (mResizeCallback)
    mResizeCallback (width, height);
  }
//}}}
//{{{
void cApp::mouse (int button, int state, int x, int y) {

  const float xf = static_cast<float>(x) / static_cast<float>(mScreenWidth);
  const float yf = static_cast<float>(y) / static_cast<float>(mScreenHeight);
  mCamera.trackball_begin_motion (xf, yf);
  }
//}}}
//{{{
void cApp::motion (int state, int x, int y) {

  const float xf = static_cast<float>(x) / static_cast<float>(mScreenWidth);
  const float yf = static_cast<float>(y) / static_cast<float>(mScreenHeight);
  if (state & SDL_BUTTON_LMASK)
    mCamera.trackball_end_motion_rotate (xf, yf);
  else if (state & SDL_BUTTON_RMASK)
    mCamera.trackball_end_motion_zoom (xf, yf);
  else if (state & SDL_BUTTON_MMASK)
    mCamera.trackball_end_motion_translate (xf, yf);
  }
//}}}
//{{{
bool cApp::processEvents() {

  bool quit = false;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent (&event);
    ImGuiIO const& io = ImGui::GetIO();
    switch (event.type) {
      case SDL_KEYDOWN:
        if (!io.WantCaptureKeyboard && mKeyboardCallback)
          mKeyboardCallback (event.key.keysym.sym);
        break;

      case SDL_KEYUP:
        break;

      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
          resize (event.window.data1, event.window.data2);
        break;

      case SDL_MOUSEBUTTONUP:
        if (!io.WantCaptureMouse)
          SDL_CaptureMouse (SDL_FALSE);
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (!io.WantCaptureMouse) {
          SDL_CaptureMouse (SDL_TRUE);
          mouse (event.button.button, event.button.state, event.button.x, event.button.y);
          }
        break;

      case SDL_MOUSEMOTION:
        if (!io.WantCaptureMouse)
          motion (event.motion.state, event.motion.x, event.motion.y);
        break;

      case SDL_QUIT:
        SDL_Quit();
        exit (EXIT_SUCCESS);
        break;
      }
    }

  return quit;
  }
//}}}
