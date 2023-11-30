// splat.cpp - splat main
//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <memory>
#include <array>
#include <exception>

#include <Eigen/Core>

#include "../common/date.h"
#include "../common/cLog.h"

#include "cSurfelModel.h"
#include "cSplatRender.h"

using namespace std;
//}}}
namespace {
  int gModel = 0;
  cSurfelModel* gSurfelModel;

  GLviz::Camera gCamera;
  unique_ptr <cSplatRender> gSplatRender;

  // callbacks
  void display() { gSplatRender->render (gSurfelModel); }
  //{{{
  void resize (int width, int height) {

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    glViewport (0, 0, width, height);
    gCamera.set_perspective (60.0f, aspect, 0.005f, 5.0f);
    }
  //}}}
  //{{{
  void gui() {

    ImGui::Begin ("GLviz", nullptr);
    ImGui::SetWindowPos (ImVec2(3.0f, 3.0f), ImGuiCond_Once);
    ImGui::SetWindowSize (ImVec2(350.0f, 415.0f), ImGuiCond_Once);

    ImGui::PushItemWidth (ImGui::GetContentRegionAvail().x * 0.55f);
    ImGui::Text ("fps \t %.1f fps", ImGui::GetIO().Framerate);

    ImGui::SetNextItemOpen (true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader ("Scene"))
      if (ImGui::Combo ("Models", &gModel, "Dragon\0Checker\0Cube\0Piccy\0\0"))
        gSurfelModel->load (gModel);

    gSplatRender->gui();

    ImGui::End();
    }
  //}}}
  //{{{
  void keyboard (SDL_Keycode key) {

    if (!gSplatRender->keyboard (key))
      switch (key) {
        case SDLK_f:
          GLviz::toggleFullScreen();
          break;

        case SDLK_q:
        case SDLK_ESCAPE:
          exit (EXIT_SUCCESS);
          break;

        default:
          break;
        }
    }
  //}}}
  void close() { gSplatRender = nullptr; }
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
  gCamera.translate (Eigen::Vector3f(0.0f, 0.0f, -2.0f));
  gSplatRender = unique_ptr<cSplatRender>(new cSplatRender (gCamera));

  gModel = 0;
  gSurfelModel = new cSurfelModel();
  gSurfelModel->load (gModel);

  GLviz::displayCallback (display);
  GLviz::resizeCallback (resize);
  GLviz::closeCallback (close);
  GLviz::guiCallback (gui);
  GLviz::keyboardCallback (keyboard);

  return GLviz::mainUILoop (gCamera);
  }
