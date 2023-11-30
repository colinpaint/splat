// splat.cpp
//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdlib>
#include <memory>
#include <vector>
#include <array>
#include <exception>
#include <cmath>

#include <Eigen/Core>

#include "../common/date.h"
#include "../common/cLog.h"

#include "cMeshRender.h"
#include "cSplatRender.h"

using namespace std;
//}}}
namespace {
  GLviz::Camera gCamera;
  cModel* gModel;
  unique_ptr<cRender> gRender;

  bool gRipple = false;
  bool gSplat = false;
  int gModelIndex = 0;

  //{{{
  void display() {
    gRender->render (gModel);
    }
  //}}}
  //{{{
  void resize (int width, int height) {

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    glViewport (0, 0, width, height);
    gCamera.set_perspective (60.0f, aspect, 0.005f, 5.0f);
    }
  //}}}
  //{{{
  void gui() {

    ImGui::Begin ("splat", nullptr);
    ImGui::SetWindowPos (ImVec2(3.0f, 3.0f), ImGuiCond_Once);
    ImGui::SetWindowSize (ImVec2(350.0f, 435.0f), ImGuiCond_Once);

    ImGui::PushItemWidth (ImGui::GetContentRegionAvail().x * 0.55f);
    ImGui::Text ("fps \t %.1f fps", ImGui::GetIO().Framerate);

    if (ImGui::Checkbox ("splat", &gSplat))
      gRender = gSplat ? unique_ptr<cRender>(new cSplatRender (gCamera)) :
                         unique_ptr<cRender>(new cMeshRender (gCamera));

    ImGui::SetNextItemOpen (true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader ("Scene"))
      if (ImGui::Combo ("Models", &gModelIndex, "DragonLo\0DragonHi\0Checker\0Cube\0Piccy\0\0"))
        gModel->load (gModelIndex);

    gRender->gui();

    ImGui::End();
    }
  //}}}
  //{{{
  void keyboard (SDL_Keycode key) {

    if (!gRender->keyboard (key))
      switch (key) {
        case SDLK_f: GLviz::toggleFullScreen(); break;
        case SDLK_SPACE: gRipple = !gRipple; break;
        case SDLK_q:
        case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;
        default: break;
        }
    }
  //}}}
  //{{{
  void timer (int delta_t_msec) {
    if (!gSplat && gRipple)
      gModel->ripple();
    }
  //}}}
  //{{{
  void close() {
    gModel = nullptr;
    gRender = nullptr;
    }
  //}}}
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
  cLog::log (LOGNOTICE, "splat");

  GLviz::init (960, 540);
  gModel = new cSurfelModel();
  gModel->load (gModelIndex);
  gCamera.translate (Eigen::Vector3f(0.0f, 0.0f, -2.0f));
  gRender = unique_ptr<cMeshRender>(new cMeshRender (gCamera));

  GLviz::displayCallback (display);
  GLviz::resizeCallback (resize);
  GLviz::closeCallback (close);
  GLviz::guiCallback (gui);
  GLviz::keyboardCallback (keyboard);

  return GLviz::mainUILoop (gCamera);
  }
