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

#include "glviz/glviz.h"
#include "glviz/buffer.h"
#include "glviz/program.h"
#include "glviz/shader.h"
#include "glviz/utility.h"

#include "../common/date.h"
#include "../common/cLog.h"

#include "cSimpleRender.h"

using namespace std;
//}}}

namespace {
  GLviz::Camera gCamera;
  unique_ptr <cSimpleRender> gSimpleRender;

  //{{{  vars
  bool g_stop_simulation(true);
  bool g_enable_mesh3(true);
  bool g_enable_wireframe(false);
  bool gEnableSpheres(false);

  vector <Eigen::Vector3f> g_ref_vertices;
  vector <Eigen::Vector3f> g_ref_normals;
  vector <Eigen::Vector3f> g_vertices;
  vector <Eigen::Vector3f> g_normals;
  vector <array <unsigned int,3>> g_faces;

  float g_time(0.0f);
  float g_point_radius(0.0014f);
  float g_projection_radius(0.0f);
  float g_wireframe[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  float g_mesh_material[4] = { 0.0f, 0.25f, 1.0f, 8.0f };
  float g_points_material[4] = { 1.0f, 1.0f, 1.0f, 8.0f };

  int g_shading_method(0);
  //}}}

  // callbacks
  //{{{
  void resize (int width, int height) {

    glViewport (0, 0, width, height);
    const float aspect = static_cast<float>(width) / static_cast<float>(height);
    gCamera.set_perspective (60.0f, aspect, 0.005f, 5.0f);
    }
  //}}}
  //{{{
  void display() {

    glEnable (GL_MULTISAMPLE);
    glEnable (GL_DEPTH_TEST);

    glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth (1.0f);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gSimpleRender->vertex_array_buffer.set_buffer_data (3 * sizeof(GLfloat) * g_vertices.size(), g_vertices.front().data());
    gSimpleRender->normal_array_buffer.set_buffer_data (3 * sizeof(GLfloat) * g_normals.size(), g_normals.front().data());
    gSimpleRender->index_array_buffer.set_buffer_data (3 * sizeof(GLuint) * g_faces.size(), g_faces.front().data());

    gSimpleRender->uniform_camera.set_buffer_data (gCamera);

    if (g_enable_mesh3) {
      gSimpleRender->uniform_material.set_buffer_data (g_mesh_material);
      gSimpleRender->program_mesh3.set_wireframe (g_enable_wireframe);
      int screen[2] = { GLviz::getScreenWidth(), GLviz::getScreenHeight() };
      gSimpleRender->uniform_wireframe.set_buffer_data (g_wireframe, screen);
      gSimpleRender->program_mesh3.set_smooth (g_shading_method != 0);
      gSimpleRender->drawMesh3 (g_shading_method, static_cast<GLsizei>(3 * g_faces.size()));
      }

    if (gEnableSpheres) {
      gSimpleRender->uniform_material.set_buffer_data (g_points_material);
      GLviz::Frustum view_frustum = gCamera.get_frustum();
      g_projection_radius = view_frustum.near_() *
                            (GLviz::getScreenHeight() / (view_frustum.top() - view_frustum.bottom()));
      gSimpleRender->uniform_sphere.set_buffer_data (g_point_radius, g_projection_radius);
      gSimpleRender->drawSpheres (static_cast<GLsizei>(g_vertices.size()));
      }
    }
  //}}}
  //{{{
  void gui() {

    ImGui::Begin ("GLviz", nullptr);
    ImGui::SetWindowPos (ImVec2(3.0f, 3.0f), ImGuiCond_Once);
    ImGui::SetWindowSize (ImVec2(265.0f, 345.0f), ImGuiCond_Once);

    ImGui::PushItemWidth (ImGui::GetContentRegionAvail().x * 0.55f);

    ImGui::Text ("time\t %.3f", g_time);
    ImGui::Text ("fps \t %.1f fps", ImGui::GetIO().Framerate);

    if (ImGui::CollapsingHeader ("Mesh", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Checkbox ("Draw Triangle Mesh", &g_enable_mesh3);
      ImGui::ColorEdit3 ("Mesh Color", g_mesh_material);
      ImGui::DragFloat ("Mesh Shinines", &g_mesh_material[3], 1e-2f, 1e-12f, 1000.0f);
      ImGui::Combo ("Mesh Shading", &g_shading_method, "Flat\0Phong\0\0");

      ImGui::Separator();
      ImGui::Checkbox ("Draw Wireframe", &g_enable_wireframe);
      ImGui::ColorEdit3 ("Mesh Color", g_wireframe);
      }

    if (ImGui::CollapsingHeader ("Spheres", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Checkbox ("Draw Spheres", &gEnableSpheres);
      ImGui::DragFloat ("Points Radius", &g_point_radius, 1e-5f, 0.0f, 0.1f, "%.4f");
      ImGui::ColorEdit3 ("Points Color", g_points_material);
      ImGui::DragFloat ("Points Shininess", &g_points_material[3], 1e-2f, 1e-12f, 1000.0f);
      }

    ImGui::End();
    }
  //}}}
  //{{{
  void keyboard (SDL_Keycode key) {

    switch (key) {
      case SDLK_1: g_enable_mesh3 = !g_enable_mesh3; break;
      case SDLK_2: gEnableSpheres = !gEnableSpheres; break;
      case SDLK_5: g_shading_method = (g_shading_method + 1) % 2; break;
      case SDLK_w: g_enable_wireframe = !g_enable_wireframe; break;

      case SDLK_r: g_time = 0.0f; break;
      case SDLK_SPACE: g_stop_simulation = !g_stop_simulation; break;

      case SDLK_f: GLviz::toggleFullScreen(); break;

      case SDLK_q:
      case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;
      }
    }
  //}}}
  //{{{
  void timer (int delta_t_msec) {

    float delta_t_sec = static_cast<float>(delta_t_msec) / 1000.0f;

    if (!g_stop_simulation) {
      g_time += delta_t_sec;

      const float k = 50.0f;
      const float a = 0.03f;
      const float v = 10.0f;
      for (unsigned int i(0); i < g_vertices.size(); ++i) {
        const float x = g_ref_vertices[i].x() + g_ref_vertices[i].y() + g_ref_vertices[i].z();
        const float u = 5.0f * (x - 0.75f * sin(2.5f * g_time));
        const float w = (a / 2.0f) * (1.0f + sin(k * x + v * g_time));
        g_vertices[i] = g_ref_vertices[i] + (exp(-u * u) * w) * g_ref_normals[i];
        }

      GLviz::setVertexNormalsFromTriangleMesh (g_vertices, g_faces, g_normals);
      }
    }
  //}}}
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

  try {
    GLviz::loadMesh ("../models/stanford_dragon_v40k_f80k.raw", g_vertices, g_faces);
    //GLviz::loadMesh ("../models/stanford_dragon_v344k_f688k.raw", g_vertices, g_faces);
    GLviz::setVertexNormalsFromTriangleMesh (g_vertices, g_faces, g_normals);
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
