#pragma once
#include <cstdlib>
#include <string>
#include <functional>
#include "cCamera.h"
#include "SDL_keycode.h"

class cApp {
public:
  cApp() {}
  virtual ~cApp() = default;

  void init (int screenWidth, int screen_height, bool fullScreen, bool multiSample);

  int getScreenWidth() const { return mScreenWidth; };
  int getScreenHeight() const { return mScreenHeight; };
  cCamera& getCamera() { return mCamera; };

  // callbacks
  void cApp::setGuiCallback (std::function<void()> guiCallback) { mGuiCallback = guiCallback; }
  void cApp::setDisplayCallback (std::function<void()> displayCallback) { mDisplayCallback = displayCallback; }
  void cApp::setCloseCallback (std::function<void()> closeCallback) { mCloseCallback = closeCallback; }
  void cApp::setTimerCallback (std::function<void()> timerCallback) { mTimerCallback = timerCallback; }
  //{{{
  void cApp::setKeyboardCallback (std::function<void (SDL_Keycode)> keyboardCallback) {
    mKeyboardCallback = keyboardCallback;
    }
  //}}}
  //{{{
  void cApp::setResizeCallback (std::function<void (int width, int height)> resizeCallback) {
    mResizeCallback = resizeCallback;
    }
  //}}}

  void toggleFullScreen();

  int mainUILoop();

  static std::string getGlErrorString (GLenum gl_error);
  static std::string getGlFramebufferStatusString (GLenum framebuffer_status);

  // vars
  int mScreenWidth = 0;
  int mScreenHeight = 0;
  bool mFullScreen = false;

  cCamera mCamera;

  unsigned int mTimerMs = 40;

  std::function<void()> mGuiCallback;
  std::function<void()> mDisplayCallback;
  std::function<void()> mCloseCallback;
  std::function<void()> mTimerCallback;
  std::function<void (SDL_Keycode)> mKeyboardCallback;
  std::function<void (int width, int height)> mResizeCallback;

private:
  void resize (int width, int height);
  void mouse (int button, int state, int x, int y);
  void motion (int state, int x, int y);
  bool processEvents();
  };
