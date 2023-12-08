//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#include <string>
#include <thread>
#include <exception>
#include <limits>

#include <Eigen/Dense>

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

#include "../common/cLog.h"

#include "cModel.h"
#include "cSurfelModel.h"

using namespace std;
//}}}

// cSurfelModel
//{{{
void cSurfelModel::loadIndex (int modelIndex) {

  switch (modelIndex) {
    case 0:
      loadRawFile ("../models/stanford_dragon_v40k_f80k.raw");
      meshToSurfel();
      break;

    case 1:
      loadRawFile ("../models/stanford_dragon_v344k_f688k.raw");
      meshToSurfel();
      break;

    case 2:
      createChecker (100,100);
      break;

    case 3:
      createCube();
      break;

    default:
      cLog::log (LOGERROR, "cSurfelModel unrecognised model index");
      break;
    }
  }
//}}}
//{{{
void cSurfelModel::loadObjFile (const string& fileName) {
  cModel::loadObjFile (fileName);
  meshToSurfel();
  }
//}}}
//{{{
void cSurfelModel::loadRawFile (const string& fileName) {
  cModel::loadRawFile (fileName);
  meshToSurfel();
  }
//}}}
//{{{
void cSurfelModel::loadPiccyFile (const string& fileName) {

  mVertices.clear();
  mFaces.clear();
  mNormals.clear();
  mRefVertices.clear();
  mRefNormals.clear();

  FILE* file = fopen (fileName.c_str(), "rb");
  if (!file) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("failed to load file {}", fileName));
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
  cLog::log (LOGINFO, fmt::format ("createPiccy {} {}x{}:{}", fileName, width, height, channels));

  mModel.resize (width * height);

  const float dw = 2.0f / width;
  const float dh = 2.0f / height;
  size_t surfelIndex = 0;
  uint32_t* rgbaPixels = (uint32_t*)pixels;
  for (size_t j = 0; j < height; ++j) {
    for (size_t i = 0; i < width; ++i) {
      mModel[surfelIndex].centre = Eigen::Vector3f (-1.0f + (dh * i), -1.0f + (dw * j), 0.0f);
      mModel[surfelIndex].major = dw * Eigen::Vector3f::UnitX(); // ellipse major axis
      mModel[surfelIndex].minor = dh * Eigen::Vector3f::UnitY(); // ellipse minor axis
      mModel[surfelIndex].clipPlane= Eigen::Vector3f::Zero(),
      mModel[surfelIndex].rgba = *(rgbaPixels++);

      // clipping planes
      if (j == 0)
        mModel[surfelIndex].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
      else if (i == 0)
        mModel[surfelIndex].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
      else if (j == height-1)
        mModel[surfelIndex].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);
      else if (i == width-1)
        mModel[surfelIndex].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
      else
        mModel[surfelIndex].clipPlane= Eigen::Vector3f::Zero();
      surfelIndex++;
      }
    }
  }
//}}}

// - private
//{{{
void cSurfelModel::hsv2rgb (float h, float s, float v, float& r, float& g, float& b) {

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
void cSurfelModel::steinerCircumEllipse (float const* v0_ptr, float const* v1_ptr, float const* v2_ptr,
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
void cSurfelModel::meshToSurfel() {

  mModel.resize (mFaces.size());

  vector<thread> threads (thread::hardware_concurrency());
  for (size_t i = 0; i < threads.size(); ++i) {
    size_t b = i * mFaces.size() / threads.size();
    size_t e = (i + 1) * mFaces.size() / threads.size();

    threads[i] = thread ([this, b, e]() {
      for (size_t j = b; j < e; ++j) {
        // face to surfel
        array <unsigned int,3> const& face = mFaces[j];
        Eigen::Vector3f v[3] = { mVertices[face[0]], mVertices[face[1]], mVertices[face[2]] };

        Eigen::Vector3f p0;
        Eigen::Vector3f t1;
        Eigen::Vector3f t2;
        steinerCircumEllipse (v[0].data(), v[1].data(), v[2].data(), p0.data(), t1.data(), t2.data());

        Eigen::Vector3f normalS = t1.cross (t2);
        Eigen::Vector3f normalT = (v[1] - v[0]).cross (v[2] - v[0]);
        if (normalT.dot (normalS) < 0.0f)
          t1.swap (t2);

        mModel[j].centre = p0;
        mModel[j].major = t1;
        mModel[j].minor = t2;
        mModel[j].clipPlane = Eigen::Vector3f::Zero();

        float h = min (abs (p0.x()) * 360.f, 360.f);
        float r, g, b;
        hsv2rgb (h, 1.0f, 1.0f, r, g, b);
        mModel[j].rgba = static_cast<uint32_t>(r * 255.f) |
                         (static_cast<uint32_t>(g * 255.f) << 8) |
                         (static_cast<uint32_t>(b * 255.f) << 16);
        }
      });
    }

  for (auto& t : threads)
    t.join();
  }
//}}}
//{{{
void cSurfelModel::createModel (const string& fileName) {

  try {
    loadRawFile (fileName);
    meshToSurfel();
    }

  catch (runtime_error const& e) {
    cLog::log (LOGERROR, fmt::format ("createModel {}", e.what()));
    exit (EXIT_FAILURE);
    }
  }
//}}}

//{{{
void cSurfelModel::createChecker (size_t width, size_t height) {

  cLog::log (LOGINFO, fmt::format ("createChecker {}x{}", width, height));

  mVertices.clear();
  mFaces.clear();
  mNormals.clear();
  mRefVertices.clear();
  mRefNormals.clear();

  const float dw = 1.0f / static_cast<float>(2 * width);
  const float dh = 1.0f / static_cast<float>(2 * height);
  cSurfel surfel (Eigen::Vector3f::Zero(),
                  2.0f * dw * Eigen::Vector3f::UnitX(),
                  2.0f * dh * Eigen::Vector3f::UnitY(),
                  Eigen::Vector3f::Zero(), 0);
  mModel.resize (4 * width * height);

  size_t m = 0;
  for (size_t i = 0; i <= 2 * width; ++i) {
    for (size_t j = 0; j <= 2 * height; ++j) {
      size_t k = i * (2 * width + 1) + j;
      if (k % 2 == 1) {
        surfel.centre = Eigen::Vector3f (-1.0f + 2.0f * dh * static_cast<float>(j),
                                         -1.0f + 2.0f * dw * static_cast<float>(i),
                                         0.0f);
        surfel.rgba = (((j / 2) % 2) == ((i / 2) % 2)) ? 0u : ~0u;
        mModel[m] = surfel;

        // Clip border surfelModel
        if (j == 0)
          mModel[m].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
        else if (i == 0)
          mModel[m].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

        else if (j == 2 * height) {
          mModel[m].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);
          mModel[m].rgba = ~surfel.rgba;
          }
        else if (i == 2 * width) {
          mModel[m].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
          mModel[m].rgba = ~surfel.rgba;
          }

        else {
          // Duplicate and clip inner surfelModel
          if (j % 2 == 0) {
            mModel[m].clipPlane= Eigen::Vector3f(1.0, 0.0f, 0.0f);
            mModel[++m] = surfel;
            mModel[m].clipPlane= Eigen::Vector3f(-1.0, 0.0f, 0.0f);
            mModel[m].rgba = ~surfel.rgba;
            }

          if (i % 2 == 0) {
            mModel[m].clipPlane= Eigen::Vector3f(0.0, 1.0f, 0.0f);
            mModel[++m] = surfel;
            mModel[m].clipPlane= Eigen::Vector3f(0.0, -1.0f, 0.0f);
            mModel[m].rgba = ~surfel.rgba;
            }
          }
        ++m;
        }
      }
    }
  }
//}}}
//{{{
void cSurfelModel::createCube() {

  mVertices.clear();
  mFaces.clear();
  mNormals.clear();
  mRefVertices.clear();
  mRefNormals.clear();

  cLog::log (LOGINFO, fmt::format ("createCube"));

  mModel.resize (24);
  unsigned int color = 0;

  //{{{  front
  mModel[0].centre  = Eigen::Vector3f(-0.5f, 0.0f, 0.5f);
  mModel[0].major = 0.5f * Eigen::Vector3f::UnitX();
  mModel[0].minor = 0.5f * Eigen::Vector3f::UnitY();
  mModel[0].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  mModel[0].rgba  = color;

  mModel[1]   = mModel[0];
  mModel[1].centre = Eigen::Vector3f(0.5f, 0.0f, 0.5f);
  mModel[1].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

  mModel[2]   = mModel[0];
  mModel[2].centre = Eigen::Vector3f(0.0f, 0.5f, 0.5f);
  mModel[2].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);

  mModel[3]   = mModel[0];
  mModel[3].centre = Eigen::Vector3f(0.0f, -0.5f, 0.5f);
  mModel[3].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
  //}}}
  //{{{  back
  mModel[4].centre = Eigen::Vector3f(-0.5f, 0.0f, -0.5f);
  mModel[4].major = 0.5f * Eigen::Vector3f::UnitX();
  mModel[4].minor = -0.5f * Eigen::Vector3f::UnitY();
  mModel[4].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  mModel[4].rgba = color;

  mModel[5] = mModel[4];
  mModel[5].centre = Eigen::Vector3f(0.5f, 0.0f, -0.5f);
  mModel[5].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

  mModel[6] = mModel[4];
  mModel[6].centre = Eigen::Vector3f(0.0f, 0.5f, -0.5f);
  mModel[6].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

  mModel[7] = mModel[4];
  mModel[7].centre = Eigen::Vector3f(0.0f, -0.5f, -0.5f);
  mModel[7].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
  //}}}
  //{{{  top
  mModel[8].centre = Eigen::Vector3f(-0.5f, 0.5f, 0.0f);
  mModel[8].major = 0.5f * Eigen::Vector3f::UnitX();
  mModel[8].minor = -0.5f * Eigen::Vector3f::UnitZ();
  mModel[8].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  mModel[8].rgba = color;

  mModel[9]    = mModel[8];
  mModel[9].centre  = Eigen::Vector3f(0.5f, 0.5f, 0.0f);
  mModel[9].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

  mModel[10]    = mModel[8];
  mModel[10].centre  = Eigen::Vector3f(0.0f, 0.5f, 0.5f);
  mModel[10].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

  mModel[11] = mModel[8];
  mModel[11].centre = Eigen::Vector3f(0.0f, 0.5f, -0.5f);
  mModel[11].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
  //}}}
  //{{{  bottom
  mModel[12].centre = Eigen::Vector3f(-0.5f, -0.5f, 0.0f);
  mModel[12].major = 0.5f * Eigen::Vector3f::UnitX();
  mModel[12].minor = 0.5f * Eigen::Vector3f::UnitZ();
  mModel[12].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  mModel[12].rgba = color;

  mModel[13] = mModel[12];
  mModel[13].centre = Eigen::Vector3f(0.5f, -0.5f, 0.0f);
  mModel[13].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

  mModel[14] = mModel[12];
  mModel[14].centre = Eigen::Vector3f(0.0f, -0.5f, 0.5f);
  mModel[14].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);

  mModel[15] = mModel[12];
  mModel[15].centre = Eigen::Vector3f(0.0f, -0.5f, -0.5f);
  mModel[15].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
  //}}}
  //{{{  left
  mModel[16].centre = Eigen::Vector3f(-0.5f, -0.5f, 0.0f);
  mModel[16].major = 0.5f * Eigen::Vector3f::UnitY();
  mModel[16].minor = -0.5f * Eigen::Vector3f::UnitZ();
  mModel[16].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  mModel[16].rgba = color;

  mModel[17] = mModel[16];
  mModel[17].centre = Eigen::Vector3f(-0.5f, 0.5f, 0.0f);
  mModel[17].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

  mModel[18] = mModel[16];
  mModel[18].centre = Eigen::Vector3f(-0.5f, 0.0f, 0.5f);
  mModel[18].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);

  mModel[19] = mModel[16];
  mModel[19].centre = Eigen::Vector3f(-0.5f, 0.0f, -0.5f);
  mModel[19].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);
  //}}}
  //{{{  right
  mModel[20].centre = Eigen::Vector3f(0.5f, -0.5f, 0.0f);
  mModel[20].major = 0.5f * Eigen::Vector3f::UnitY();
  mModel[20].minor = 0.5f * Eigen::Vector3f::UnitZ();
  mModel[20].clipPlane= Eigen::Vector3f(1.0f, 0.0f, 0.0f);
  mModel[20].rgba = color;

  mModel[21] = mModel[20];
  mModel[21].centre = Eigen::Vector3f(0.5f, 0.5f, 0.0f);
  mModel[21].clipPlane= Eigen::Vector3f(-1.0f, 0.0f, 0.0f);

  mModel[22] = mModel[20];
  mModel[22].centre = Eigen::Vector3f(0.5f, 0.0f, 0.5f);
  mModel[22].clipPlane= Eigen::Vector3f(0.0f, -1.0f, 0.0f);

  mModel[23] = mModel[20];
  mModel[23].centre = Eigen::Vector3f(0.5f, 0.0f, -0.5f);
  mModel[23].clipPlane= Eigen::Vector3f(0.0f, 1.0f, 0.0f);
  //}}}
  }
//}}}
