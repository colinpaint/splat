#pragma once
#include "camera.h"

#include <imgui.h>
#include "SDL_keycode.h"

namespace GLviz {
  int      screen_width();
  int      screen_height();

  Camera*  camera();
  void     set_camera (Camera& camera);

  void     displayCallback (std::function<void()> displayCallback);
  void     resizeCallback (std::function<void (int, int)> resizeCallback);
  void     timerCallback (std::function<void (unsigned int)> timerCallback, unsigned int timer_msec);
  void     closeCallback (std::function<void()> closeCallback);
  void     guiCallback (std::function<void()> guiCallback);
  void     keyboardCallback (std::function<void (SDL_Keycode)> keyboardCallback);

  void     cout_opengl_version();
  void     cout_glew_version();

  void     GLviz (int screen_width = 960, int screen_height = 540);
  int      exec (Camera& camera);
  }
