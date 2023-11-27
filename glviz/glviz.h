#pragma once
#include "camera.h"

#include <imgui.h>
#include "SDL_keycode.h"

namespace GLviz {
  int      screen_width();
  int      screen_height();

  Camera*  camera();
  void     set_camera(Camera& camera);

  void     display_callback(std::function<void ()> display_callback);
  void     reshape_callback(std::function<void (int, int)> reshape_callback);
  void     timer_callback(std::function<void (unsigned int)> timer_callback,
               unsigned int timer_msec);
  void     close_callback(std::function<void ()> close_callback);
  void     gui_callback(std::function<void ()> gui_callback);
  void     keyboard_callback(std::function<void (SDL_Keycode)>
               keyboard_callback);

  void     cout_opengl_version();
  void     cout_glew_version();

  void     GLviz(int screen_width = 960, int screen_height = 540);
  int      exec(Camera& camera);
  }
