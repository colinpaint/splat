// splat.cpp - splat main
//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <exception>
#include <thread>

#include <Eigen/Core>

#include "glviz/glviz.h"
#include "glviz/utility.h"

//{{{  include stb
// invoke header only library implementation here
#ifdef _WIN32
  #pragma warning (push)
  #pragma warning (disable: 4244)
#endif

  #define STB_IMAGE_IMPLEMENTATION
  #include "stb/stb_image.h"

#ifdef _WIN32
  #pragma warning (pop)
#endif
//}}}

#include "../common/date.h"
#include "../common/cLog.h"

#include "cSplatRender.h"

using namespace std;
//}}}

namespace {
  GLviz::Camera gCamera;
  vector <sSurfel> gSurfels;
  unique_ptr <cSplatRender> gSplatRender;

  int gModel = 0;

  // model
  //{{{
  void hsv2rgb (float h, float s, float v, float& r, float& g, float& b) {

    float h_i = floor(h / 60.0f);
    float f = h / 60.0f - h_i;

    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (static_cast<int>(h_i)) {
      case 1:
        r = q; g = v; b = p;
        break;
      case 2:
        r = p; g = v; b = t;
        break;
      case 3:
        r = p; g = q; b = v;
        break;
      case 4:
        r = t; g = p; b = v;
        break;
      case 5:
        r = v; g = p; b = q;
        break;
      default:
        r = v; g = t; b = p;
      }
    }
  //}}}
  //{{{
  void steinerCircumEllipse (float const* v0_ptr, float const* v1_ptr, float const* v2_ptr,
                             float* p0_ptr, float* t1_ptr, float* t2_ptr) {

    Eigen::Matrix2f Q;
    Eigen::Vector3f d0, d1, d2;

      {
      using Vec = Eigen::Map<const Eigen::Vector3f>;
      Vec v[] = { Vec(v0_ptr), Vec(v1_ptr), Vec(v2_ptr) };

      d0 = v[1] - v[0];
      d0.normalize();

      d1 = v[2] - v[0];
      d1 = d1 - d0 * d0.dot(d1);
      d1.normalize();

      d2 = (1.0f / 3.0f) * (v[0] + v[1] + v[2]);

      Eigen::Vector2f p[3];
      for (unsigned int j = 0; j < 3; ++j)
        p[j] = Eigen::Vector2f (d0.dot(v[j] - d2), d1.dot(v[j] - d2));

      Eigen::Matrix3f A;
      for (unsigned int j = 0; j < 3; ++j)
        A.row(j) = Eigen::Vector3f (p[j].x() * p[j].x(), 2.0f * p[j].x() * p[j].y(), p[j].y() * p[j].y());

      Eigen::FullPivLU<Eigen::Matrix3f> lu(A);
      Eigen::Vector3f res = lu.solve (Eigen::Vector3f::Ones());

      Q(0, 0) = res(0);
      Q(1, 1) = res(2);
      Q(0, 1) = Q(1, 0) = res(1);
      }

    Eigen::Map <Eigen::Vector3f> p0(p0_ptr), t1(t1_ptr), t2(t2_ptr);

      {
      Eigen::SelfAdjointEigenSolver <Eigen::Matrix2f> es;
      es.compute (Q);

      Eigen::Vector2f const& l = es.eigenvalues();
      Eigen::Vector2f const& e0 = es.eigenvectors().col(0);
      Eigen::Vector2f const& e1 = es.eigenvectors().col(1);

      p0 = d2;
      t1 = (1.0f / sqrt(l.x())) * (d0 * e0.x() + d1 * e0.y());
      t2 = (1.0f / sqrt(l.y())) * (d0 * e1.x() + d1 * e1.y());
      }
    }
  //}}}
  //{{{
  void meshToSurfel (vector <Eigen::Vector3f> const& vertices,
                     vector <array <unsigned int, 3>> const& faces,
                     vector<sSurfel>& surfels) {

    surfels.resize (faces.size());

    vector<thread> threads (thread::hardware_concurrency());
    for (size_t i = 0; i < threads.size(); ++i) {
      size_t b = i * faces.size() / threads.size();
      size_t e = (i + 1) * faces.size() / threads.size();

      threads[i] = thread ([b, e, &vertices, &faces, &surfels]() {
        for (size_t j = b; j < e; ++j) {
          // face to surfel
          array <unsigned int,3> const& face = faces[j];
          Eigen::Vector3f v[3] = { vertices[face[0]], vertices[face[1]], vertices[face[2]] };
          sSurfel& surfel = surfels[j];

          Eigen::Vector3f p0;
          Eigen::Vector3f t1;
          Eigen::Vector3f t2;
          steinerCircumEllipse (v[0].data(), v[1].data(), v[2].data(), p0.data(), t1.data(), t2.data());

          Eigen::Vector3f normalS = t1.cross (t2);
          Eigen::Vector3f normalT = (v[1] - v[0]).cross (v[2] - v[0]);
          if (normalT.dot (normalS) < 0.0f)
            t1.swap (t2);

          surfel.centre = p0;
          surfel.major = t1;
          surfel.minor = t2;
          surfel.clipPlane= Eigen::Vector3f::Zero();

          float h = min((abs(p0.x()) / 0.45f) * 360.0f, 360.0f);
          float r, g, b;
          hsv2rgb (h, 1.0f, 1.0f, r, g, b);
          surfel.rgba = static_cast<uint32_t>(r * 255.0f)
                      | (static_cast<uint32_t>(g * 255.0f) << 8)
                      | (static_cast<uint32_t>(b * 255.0f) << 16);
          }
        });
      }

    for (auto& t : threads)
      t.join();
    }
  //}}}
  //{{{
  void createModel (const string& filename) {

    try {
      vector <Eigen::Vector3f> vertices;
      vector <array <unsigned int,3>> faces;
      GLviz::loadMesh (filename, vertices, faces);

      vector <Eigen::Vector3f> normals;
      GLviz::setVertexNormalsFromTriangleMesh (vertices, faces, normals);

      meshToSurfel (vertices, faces, gSurfels);
      }

    catch (runtime_error const& e) {
      cerr << e.what() << endl;
      exit (EXIT_FAILURE);
      }
    }
  //}}}
  //{{{
  void createChecker (size_t width, size_t height) {

    cLog::log (LOGINFO, fmt::format ("createChecker {}x{}", width, height));

    const float dw = 1.0f / static_cast<float>(2 * width);
    const float dh = 1.0f / static_cast<float>(2 * height);
    sSurfel surfel (Eigen::Vector3f::Zero(),
                    2.0f * dw * Eigen::Vector3f::UnitX(),
                    2.0f * dh * Eigen::Vector3f::UnitY(),
                    Eigen::Vector3f::Zero(),
                    0);

    gSurfels.resize (4 * width * height);

    size_t m = 0;
    for (size_t i = 0; i <= 2 * width; ++i) {
      for (size_t j = 0; j <= 2 * height; ++j) {
        size_t k = i * (2 * width + 1) + j;
        if (k % 2 == 1) {
          surfel.centre = Eigen::Vector3f (-1.0f + 2.0f * dh * static_cast<float>(j),
                                           -1.0f + 2.0f * dw * static_cast<float>(i),
                                           0.0f);
          surfel.rgba = (((j / 2) % 2) == ((i / 2) % 2)) ? 0u : ~0u;
          gSurfels[m] = surfel;

          // Clip border surfels
          if (j == 0)
            gSurfels[m].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
          else if (i == 0)
            gSurfels[m].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

          else if (j == 2 * height) {
            gSurfels[m].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);
            gSurfels[m].rgba = ~surfel.rgba;
            }
          else if (i == 2 * width) {
            gSurfels[m].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
            gSurfels[m].rgba = ~surfel.rgba;
            }

          else {
            // Duplicate and clip inner surfels
            if (j % 2 == 0) {
              gSurfels[m].clipPlane= Eigen::Vector3f(1.0, 0.0f, 0.0f);
              gSurfels[++m] = surfel;
              gSurfels[m].clipPlane= Eigen::Vector3f(-1.0, 0.0f, 0.0f);
              gSurfels[m].rgba = ~surfel.rgba;
              }

            if (i % 2 == 0) {
              gSurfels[m].clipPlane= Eigen::Vector3f(0.0, 1.0f, 0.0f);
              gSurfels[++m] = surfel;
              gSurfels[m].clipPlane= Eigen::Vector3f(0.0, -1.0f, 0.0f);
              gSurfels[m].rgba = ~surfel.rgba;
              }
            }
          ++m;
          }
        }
      }
    }
  //}}}
  //{{{
  void createPiccy (const string& filename) {

    FILE* file = fopen (filename.c_str(), "rb");
    if (!file) {
      //{{{  error, return
      cLog::log (LOGERROR, fmt::format ("failed to load file {}", filename));
      return;
      }
      //}}}

    constexpr uint32_t kMaxFileSize = 20000000;
    uint8_t* fileBuf = new uint8_t [kMaxFileSize];
    uint32_t fileBufLen = 0;
    fileBufLen = (uint32_t)fread (fileBuf, 1, kMaxFileSize, file);
    fclose (file);

    int32_t width;
    int32_t height;
    int32_t channels;
    uint8_t* pixels = (uint8_t*)stbi_load_from_memory (fileBuf, fileBufLen, &width, &height, &channels, 4);

    cLog::log (LOGINFO, fmt::format ("createPiccy {} {}x{}:{}", filename, width, height, channels));

    gSurfels.resize (width * height);

    const float dw = 2.0f / width;
    const float dh = 2.0f / height;

    size_t surfelIndex = 0;
    uint32_t* rgbaPixels = (uint32_t*)pixels;
    for (size_t j = 0; j < height; ++j) {
      for (size_t i = 0; i < width; ++i) {
        gSurfels[surfelIndex].centre = Eigen::Vector3f (-1.0f + (dh * i), -1.0f + (dw * j), 0.0f);
        gSurfels[surfelIndex].major = dw * Eigen::Vector3f::UnitX(); // ellipse major axis
        gSurfels[surfelIndex].minor = dh * Eigen::Vector3f::UnitY(); // ellipse minor axis
        gSurfels[surfelIndex].clipPlane= Eigen::Vector3f::Zero(),
        gSurfels[surfelIndex].rgba = *(rgbaPixels++);

        // clipping planes
        if (j == 0)
          gSurfels[surfelIndex].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
        else if (i == 0)
          gSurfels[surfelIndex].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
        else if (j == height-1)
          gSurfels[surfelIndex].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);
        else if (i == width-1)
          gSurfels[surfelIndex].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
        else
          gSurfels[surfelIndex].clipPlane= Eigen::Vector3f::Zero();
        surfelIndex++;
        }
      }
    }
  //}}}
  //{{{
  void createCube() {

    cLog::log (LOGINFO, fmt::format ("createCube"));

    sSurfel cube[24];
    unsigned int color = 0;

    //{{{  front
    cube[0].centre  = Eigen::Vector3f(-0.5f, 0.0f, 0.5f);
    cube[0].major = 0.5f * Eigen::Vector3f::UnitX();
    cube[0].minor = 0.5f * Eigen::Vector3f::UnitY();
    cube[0].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    cube[0].rgba  = color;

    cube[1]   = cube[0];
    cube[1].centre = Eigen::Vector3f(0.5f, 0.0f, 0.5f);
    cube[1].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

    cube[2]   = cube[0];
    cube[2].centre = Eigen::Vector3f(0.0f, 0.5f, 0.5f);
    cube[2].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);

    cube[3]   = cube[0];
    cube[3].centre = Eigen::Vector3f(0.0f, -0.5f, 0.5f);
    cube[3].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
    //}}}
    //{{{  back
    cube[4].centre = Eigen::Vector3f(-0.5f, 0.0f, -0.5f);
    cube[4].major = 0.5f * Eigen::Vector3f::UnitX();
    cube[4].minor = -0.5f * Eigen::Vector3f::UnitY();
    cube[4].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    cube[4].rgba = color;

    cube[5] = cube[4];
    cube[5].centre = Eigen::Vector3f(0.5f, 0.0f, -0.5f);
    cube[5].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

    cube[6] = cube[4];
    cube[6].centre = Eigen::Vector3f(0.0f, 0.5f, -0.5f);
    cube[6].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

    cube[7] = cube[4];
    cube[7].centre = Eigen::Vector3f(0.0f, -0.5f, -0.5f);
    cube[7].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
    //}}}
    //{{{  top
    cube[8].centre = Eigen::Vector3f(-0.5f, 0.5f, 0.0f);
    cube[8].major = 0.5f * Eigen::Vector3f::UnitX();
    cube[8].minor = -0.5f * Eigen::Vector3f::UnitZ();
    cube[8].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    cube[8].rgba = color;

    cube[9]    = cube[8];
    cube[9].centre  = Eigen::Vector3f(0.5f, 0.5f, 0.0f);
    cube[9].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

    cube[10]    = cube[8];
    cube[10].centre  = Eigen::Vector3f(0.0f, 0.5f, 0.5f);
    cube[10].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

    cube[11] = cube[8];
    cube[11].centre = Eigen::Vector3f(0.0f, 0.5f, -0.5f);
    cube[11].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
    //}}}
    //{{{  bottom
    cube[12].centre = Eigen::Vector3f(-0.5f, -0.5f, 0.0f);
    cube[12].major = 0.5f * Eigen::Vector3f::UnitX();
    cube[12].minor = 0.5f * Eigen::Vector3f::UnitZ();
    cube[12].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    cube[12].rgba = color;

    cube[13] = cube[12];
    cube[13].centre = Eigen::Vector3f(0.5f, -0.5f, 0.0f);
    cube[13].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

    cube[14] = cube[12];
    cube[14].centre = Eigen::Vector3f(0.0f, -0.5f, 0.5f);
    cube[14].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);

    cube[15] = cube[12];
    cube[15].centre = Eigen::Vector3f(0.0f, -0.5f, -0.5f);
    cube[15].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
    //}}}
    //{{{  left
    cube[16].centre = Eigen::Vector3f(-0.5f, -0.5f, 0.0f);
    cube[16].major = 0.5f * Eigen::Vector3f::UnitY();
    cube[16].minor = -0.5f * Eigen::Vector3f::UnitZ();
    cube[16].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    cube[16].rgba = color;

    cube[17] = cube[16];
    cube[17].centre = Eigen::Vector3f(-0.5f, 0.5f, 0.0f);
    cube[17].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

    cube[18] = cube[16];
    cube[18].centre = Eigen::Vector3f(-0.5f, 0.0f, 0.5f);
    cube[18].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

    cube[19] = cube[16];
    cube[19].centre = Eigen::Vector3f(-0.5f, 0.0f, -0.5f);
    cube[19].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
    //}}}
    //{{{  right
    cube[20].centre = Eigen::Vector3f(0.5f, -0.5f, 0.0f);
    cube[20].major = 0.5f * Eigen::Vector3f::UnitY();
    cube[20].minor = 0.5f * Eigen::Vector3f::UnitZ();
    cube[20].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
    cube[20].rgba = color;

    cube[21] = cube[20];
    cube[21].centre = Eigen::Vector3f(0.5f, 0.5f, 0.0f);
    cube[21].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

    cube[22] = cube[20];
    cube[22].centre = Eigen::Vector3f(0.5f, 0.0f, 0.5f);
    cube[22].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);

    cube[23] = cube[20];
    cube[23].centre = Eigen::Vector3f(0.5f, 0.0f, -0.5f);
    cube[23].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
    //}}}

    gSurfels = vector <sSurfel> (cube, cube + 24);
    }
  //}}}
  //{{{
  void loadModel (int model) {

    switch (model) {
      case 0:
        createModel ("../models/stanford_dragon_v40k_f80k.raw");
        //createModel ("../models/stanford_dragon_v344k_f688k.raw");
        break;

      case 1:
        createChecker (100,100);
        break;

      case 2:
        createCube();
        break;

      case 3: {
        createPiccy ("../piccies/test.png");
        break;
        }
      }
    }
  //}}}

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
        loadModel (gModel);

    ImGui::SetNextItemOpen (true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader ("Surface Splatting")) {
      int shadingMethod = gSplatRender->smooth() ? 1 : 0;
      if (ImGui::Combo ("Shading", &shadingMethod, "Flat\0Smooth\0\0"))
        gSplatRender->set_smooth (shadingMethod > 0 ? true : false);

      //{{{  material
      ImGui::Separator();
      int color_material = gSplatRender->color_material() ? 1 : 0;
      if (ImGui::Combo ("Color", &color_material, "Surfel\0Material\0\0"))
        gSplatRender->set_color_material (color_material > 0 ? true : false);

      float material_color[3];
      copy(gSplatRender->material_color(), gSplatRender->material_color() + 3, material_color);
      if (ImGui::ColorEdit3 ("Material color", material_color))
        gSplatRender->set_material_color (material_color);

      float material_shininess = gSplatRender->material_shininess();
      if (ImGui::DragFloat ("Material shininess", &material_shininess, 0.05f, 1e-12f, 1000.0f))
        gSplatRender->set_material_shininess (min(max( 1e-12f, material_shininess), 1000.0f));
      //}}}
      //{{{  soft z
      ImGui::Separator();
      bool soft_zbuffer = gSplatRender->soft_zbuffer();
      if (ImGui::Checkbox("Soft z-buffer", &soft_zbuffer))
        gSplatRender->set_soft_zbuffer (soft_zbuffer);

      float soft_zbuffer_epsilon = gSplatRender->soft_zbuffer_epsilon();
      if (ImGui::DragFloat ("Soft z-buffer epsilon", &soft_zbuffer_epsilon, 1e-5f, 1e-5f, 1.0f, "%.5f"))
        gSplatRender->set_soft_zbuffer_epsilon (min(max(1e-5f, soft_zbuffer_epsilon), 1.0f));
      //}}}
      //{{{  ewa
      ImGui::Separator();
      bool ewa_filter = gSplatRender->ewa_filter();
      if (ImGui::Checkbox ("EWA filter", &ewa_filter))
        gSplatRender->set_ewa_filter (ewa_filter);

      float ewa_radius = gSplatRender->ewa_radius();
      if (ImGui::DragFloat ("EWA radius", &ewa_radius, 1e-3f, 0.1f, 4.0f))
        gSplatRender->set_ewa_radius (ewa_radius);
      //}}}

      ImGui::Separator();
      int point_size = gSplatRender->pointsize_method();
      if (ImGui::Combo ("Point size", &point_size, "PBP\0BHZK05\0WHA+07\0ZRB+04\0\0"))
        gSplatRender->set_pointsize_method (point_size);

      float radius_scale = gSplatRender->radius_scale();
      if (ImGui::DragFloat ("Radius scale", &radius_scale, 0.001f, 1e-6f, 2.0f))
        gSplatRender->set_radius_scale (min(max( 1e-6f, radius_scale), 2.0f));

      ImGui::Separator();
      bool multisample_4x = gSplatRender->multisample();
      if (ImGui::Checkbox ("Multisample 4x", &multisample_4x))
        gSplatRender->set_multisample (multisample_4x);

      bool backface_culling = gSplatRender->backface_culling();
      if (ImGui::Checkbox ("Backface culling", &backface_culling))
        gSplatRender->set_backface_culling (backface_culling);
      }

    ImGui::End();
    }
  //}}}
  //{{{
  void keyboard (SDL_Keycode key) {

    switch (key) {
      case SDLK_5: gSplatRender->set_smooth (!gSplatRender->smooth()); break;
      case SDLK_c: gSplatRender->set_color_material (!gSplatRender->color_material()); break;
      case SDLK_z: gSplatRender->set_soft_zbuffer (!gSplatRender->soft_zbuffer()); break;
      case SDLK_u: gSplatRender->set_ewa_filter (!gSplatRender->ewa_filter()); break;
      case SDLK_t: gSplatRender->set_pointsize_method ((gSplatRender->pointsize_method() + 1) % 4); break;

      case SDLK_f: GLviz::toggleFullScreen(); break;

      case SDLK_q:
      case SDLK_ESCAPE: exit (EXIT_SUCCESS); break;
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

  loadModel (gModel);

  GLviz::displayCallback (display);
  GLviz::resizeCallback (resize);
  GLviz::closeCallback (close);
  GLviz::guiCallback (gui);
  GLviz::keyboardCallback (keyboard);

  return GLviz::mainUILoop (gCamera);
  }
