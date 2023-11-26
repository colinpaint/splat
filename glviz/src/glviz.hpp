//{{{
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
//}}}
#pragma once
#include "camera.hpp"

#include <imgui.h>
#include <SDL2/SDL_keycode.h>

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

  void     GLviz(int screen_width = 1920, int screen_height = 1080);
  int      exec(Camera& camera);
  }
