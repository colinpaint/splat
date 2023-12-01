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
  int gModelIndex = 0;
  bool gRipple = false;

  cSplatRender* gSplatRender;
  cMeshRender* gMeshRender;
  cRender* gRender;
  bool gUseSplatRender = false;

  //{{{
  void display() {
    gRender->display (gModel);
    }
  //}}}
  //{{{
  void gui() {

    ImGui::Begin ("splat", nullptr);

    if (ImGui::Checkbox ("splatRender", &gUseSplatRender)) {
      if (gUseSplatRender)
        gRender = gSplatRender;
      else
        gRender = gMeshRender;
      gRender->bindUniforms();
      }

    bool multiSample = gRender->getMultiSample();
    if (ImGui::Checkbox ("multiSample", &multiSample))
      gRender->setMultiSample (multiSample);

    bool backFaceCull = gRender->getBackFaceCull();
    if (ImGui::Checkbox ("backfaceCull", &backFaceCull))
      gRender->setBackFaceCull (backFaceCull);

    ImGui::Text ("%.1f fps", ImGui::GetIO().Framerate);

    ImGui::PushItemWidth (0.7f * ImGui::GetContentRegionAvail().x);

    ImGui::SetNextItemOpen (true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader ("scene"))
      if (ImGui::Combo ("model", &gModelIndex, "dragonLo\0dragonHi\0checker\0cube\0piccy\0\0"))
        gModel->load (gModelIndex);

    gRender->gui();

    ImGui::End();
    }
  //}}}
  //{{{
  void keyboard (SDL_Keycode key) {

    if (!gRender->keyboard (key))
      switch (key) {
        case SDLK_f:      GLviz::toggleFullScreen(); break;

        case SDLK_SPACE:  gRipple = !gRipple; break;

        case SDLK_q:
        case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;

        default: break;
        }
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
  void timer (int delta_t_msec) {
    if (!gUseSplatRender && gRipple)
      gModel->ripple();
    }
  //}}}
  //{{{
  void close() {
    delete gMeshRender;
    delete gSplatRender;
    delete gModel;
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

  gSplatRender = new cSplatRender (gCamera);
  gMeshRender = new cMeshRender (gCamera);
  gRender = gMeshRender;

  GLviz::displayCallback (display);
  GLviz::guiCallback (gui);
  GLviz::keyboardCallback (keyboard);
  GLviz::resizeCallback (resize);
  GLviz::timerCallback (timer, 40);
  GLviz::closeCallback (close);

  return GLviz::mainUILoop (gCamera);
  }
