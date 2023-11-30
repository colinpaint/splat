// splat.cpp - splat main
//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <memory>
#include <array>
#include <exception>

#include <Eigen/Core>

#include "glviz/glviz.h"
#include "glviz/utility.h"

#include "../common/date.h"
#include "../common/cLog.h"

#include "cSplatRender.h"

using namespace std;
//}}}
namespace {
  int gModel = 0;
  cSurfels gSurfels;

  GLviz::Camera gCamera;
  unique_ptr <cSplatRender> gSplatRender;

  // callbacks
  void display() { gSplatRender->render (gSurfels); }
  //{{{
  void resize (int width, int height) {

    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    glViewport (0, 0, width, height);
    gCamera.set_perspective (60.0f, aspect, 0.005f, 5.0f);
    }
  //}}}
  //{{{
  void gui() {

    ImGui::Begin ("Surface Splatting", nullptr);
    ImGui::SetWindowPos (ImVec2(3.0f, 3.0f), ImGuiCond_Once);
    ImGui::SetWindowSize (ImVec2(350.0f, 415.0f), ImGuiCond_Once);

    ImGui::PushItemWidth (ImGui::GetContentRegionAvail().x * 0.55f);
    ImGui::Text ("fps \t %.1f fps", ImGui::GetIO().Framerate);

    ImGui::SetNextItemOpen (true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader ("Scene"))
      if (ImGui::Combo ("Models", &gModel, "Dragon\0Checker\0Cube\0Piccy\0\0"))
        gSurfels.loadModel (gModel);

    ImGui::SetNextItemOpen (true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader ("Surface Splatting")) {
      int shadingMethod = gSplatRender-> smooth() ? 1 : 0;
      if (ImGui::Combo ("Shading", &shadingMethod, "Flat\0Smooth\0\0"))
        gSplatRender->set_smooth (shadingMethod > 0 ? true : false);

      //{{{  material
      ImGui::Separator();
      int color_material = gSplatRender->getColorMaterial() ? 1 : 0;
      if (ImGui::Combo ("Color", &color_material, "Surfel\0Material\0\0"))
        gSplatRender->setColorMaterial (color_material > 0 ? true : false);

      float materialColor[3] =  { gSplatRender->getMaterialColor()[0],
                                  gSplatRender->getMaterialColor()[1],
                                  gSplatRender->getMaterialColor()[2] };
      if (ImGui::ColorEdit3 ("Material color", materialColor))
        gSplatRender->setMaterialColor (Eigen::Vector3f(materialColor[0], materialColor[1], materialColor[2]));

      float materialShininess = gSplatRender->getMaterialShininess();
      if (ImGui::DragFloat ("Material shininess", &materialShininess, 0.05f, 1e-12f, 1000.0f))
        gSplatRender->setMaterialShininess (min(max( 1e-12f, materialShininess), 1000.0f));
      //}}}
      //{{{  soft z
      ImGui::Separator();
      bool softZbuffer = gSplatRender->soft_zbuffer();
      if (ImGui::Checkbox("Soft z-buffer", &softZbuffer))
        gSplatRender->set_soft_zbuffer (softZbuffer);

      float soft_zbuffer_epsilon = gSplatRender->soft_zbuffer_epsilon();
      if (ImGui::DragFloat ("Soft z-buffer epsilon", &soft_zbuffer_epsilon, 1e-5f, 1e-5f, 1.0f, "%.5f"))
        gSplatRender->set_soft_zbuffer_epsilon (min(max(1e-5f, soft_zbuffer_epsilon), 1.0f));
      //}}}
      //{{{  ewa
      ImGui::Separator();
      bool ewaFilter = gSplatRender->ewa_filter();
      if (ImGui::Checkbox ("EWA filter", &ewaFilter))
        gSplatRender->set_ewa_filter (ewaFilter);

      float ewaRadius = gSplatRender->ewa_radius();
      if (ImGui::DragFloat ("EWA radius", &ewaRadius, 1e-3f, 0.1f, 4.0f))
        gSplatRender->set_ewa_radius (ewaRadius);
      //}}}

      ImGui::Separator();
      int pointSize = gSplatRender->pointsize_method();
      if (ImGui::Combo ("Point size", &pointSize, "PBP\0BHZK05\0WHA+07\0ZRB+04\0\0"))
        gSplatRender->set_pointsize_method (pointSize);

      float radiusScale = gSplatRender->radius_scale();
      if (ImGui::DragFloat ("Radius scale", &radiusScale, 0.001f, 1e-6f, 2.0f))
        gSplatRender->set_radius_scale (min(max( 1e-6f, radiusScale), 2.0f));

      ImGui::Separator();
      bool multiSample = gSplatRender->getMultiSample();
      if (ImGui::Checkbox ("MultiSample x4", &multiSample))
        gSplatRender->setMultiSample (multiSample);

      bool backFaceCull = gSplatRender->getBackFaceCull();
      if (ImGui::Checkbox ("Backface cull", &backFaceCull))
        gSplatRender->setBackFaceCull (backFaceCull);
      }

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

  gSurfels.loadModel (gModel);

  GLviz::displayCallback (display);
  GLviz::resizeCallback (resize);
  GLviz::closeCallback (close);
  GLviz::guiCallback (gui);
  GLviz::keyboardCallback (keyboard);

  return GLviz::mainUILoop (gCamera);
  }
