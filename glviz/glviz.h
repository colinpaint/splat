#pragma once
#include "cCamera.h"

#include <imgui.h>
#include "SDL_keycode.h"

namespace GLviz {
  // singleton statics, so use namespace
  cCamera* camera();

  void init (int screenWidth, int screen_height);

  int getScreenWidth();
  int getScreenHeight();

  void setCamera (cCamera& camera);
  void toggleFullScreen();

  void displayCallback (std::function<void()> displayCallback);
  void resizeCallback (std::function<void (int, int)> resizeCallback);
  void timerCallback (std::function<void (unsigned int)> timerCallback, unsigned int timer_msec);
  void closeCallback (std::function<void()> closeCallback);
  void guiCallback (std::function<void()> guiCallback);
  void keyboardCallback (std::function<void (SDL_Keycode)> keyboardCallback);

  int mainUILoop (cCamera& camera);
  }
