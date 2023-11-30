// simple.cpp
//{{{  includes
#include <cstdlib>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <exception>
#include <cmath>

#include <Eigen/Core>

#include "../common/date.h"
#include "../common/cLog.h"

#include "cModel.h"
#include "cSimpleRender.h"

using namespace std;
//}}}

namespace {
  GLviz::Camera gCamera;
  unique_ptr<cSimpleRender> gSimpleRender;
  cModel* gModel;
  bool gRipple = false;

  // callbacks
  //{{{
  void resize (int width, int height) {

    glViewport (0, 0, width, height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    gCamera.set_perspective (60.0f, aspect, 0.005f, 5.0f);
    }
  //}}}
  void display() { gSimpleRender->render (gModel); }
  //{{{
  void gui() {

    ImGui::Begin ("GLviz", nullptr);
    ImGui::SetWindowPos (ImVec2(3.0f, 3.0f), ImGuiCond_Once);
    ImGui::SetWindowSize (ImVec2(350.0f, 415.0f), ImGuiCond_Once);

    ImGui::PushItemWidth (ImGui::GetContentRegionAvail().x * 0.55f);
    ImGui::Text ("fps \t %.1f fps", ImGui::GetIO().Framerate);

    gSimpleRender->gui();

    ImGui::End();
    }
  //}}}
  //{{{
  void keyboard (SDL_Keycode key) {

    if (!gSimpleRender->keyboard (key))
      switch (key) {
        case SDLK_SPACE: gRipple = !gRipple; break;
        case SDLK_f: GLviz::toggleFullScreen(); break;
        case SDLK_q:
        case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;
        }
    }
  //}}}
  void timer (int delta_t_msec) { if (gRipple) gModel->ripple(); }
  void close() { gSimpleRender = nullptr; }
  }

int main (int numArgs, char* args[]) {
  eLogLevel logLevel = LOGINFO;
  //{{{  parse commandLine to params
  // parse params
  for (int i = 1; i < numArgs; i++) {
    string param = args[i];

    if (param == "log1")
      logLevel = LOGINFO1;
    else if (param == "log2")
      logLevel = LOGINFO2;
    else if (param == "log3")
      logLevel = LOGINFO3;
    }
  //}}}
  cLog::init (logLevel);
  cLog::log (LOGNOTICE, "simple");

  GLviz::init();
  gCamera.translate (Eigen::Vector3f(0.0f, 0.0f, -2.0f));

  gSimpleRender = unique_ptr<cSimpleRender>(new cSimpleRender (gCamera));

  gModel = new cModel();
  try {
    gModel->load ("../models/stanford_dragon_v40k_f80k.raw");
    //gModel.load ("../models/stanford_dragon_v344k_f688k.raw");
    }
  catch(runtime_error const& e) {
    cLog::log (LOGERROR, e.what());
    exit (EXIT_FAILURE);
    }

  GLviz::displayCallback (display);
  GLviz::resizeCallback (resize);
  GLviz::timerCallback (timer, 15);
  GLviz::closeCallback (close);
  GLviz::guiCallback (gui);
  GLviz::keyboardCallback (keyboard);

  return GLviz::mainUILoop (gCamera);
  }
