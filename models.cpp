//{{{  includes
#ifdef _WIN32
  #define _CRT_SECURE_NO_WARNINGS
#endif

#include <fstream>
#include <sstream>
#include <thread>
#include <exception>

#include <Eigen/Core>
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

#include "objLoader.h"
#include "../common/cLog.h"

#include "models.h"

using namespace std;
//}}}

// cModel
//{{{
void cModel::load (int modelIndex) {

  if (modelIndex == 0)
    loadRawFile ("../models/stanford_dragon_v40k_f80k.raw");
  else
    loadRawFile ("../models/stanford_dragon_v344k_f688k.raw");
  }
//}}}
//{{{
void cModel::loadRawFile (string const& fileName) {

  ifstream inputFind (fileName);
    if (!inputFind.good()) {
      //{{{  error, return
       cLog::log (LOGERROR, fmt::format ("cModel::loadFile - cannot find {}", fileName));
      throw runtime_error ("cannot find");
      return;
      }
      //}}}

  inputFind.close();

  ifstream input (fileName, ios::in | ios::binary);
  if (input.fail()) {
    ostringstream error_message;
    cLog::log (LOGERROR, fmt::format ("cModel::loadFile - cannot open {}", fileName));
    throw runtime_error (error_message.str().c_str());
    }

  unsigned int nv;
  input.read (reinterpret_cast<char*>(&nv), sizeof(unsigned int));
  mVertices.resize (nv);
  for (size_t i = 0; i < nv; ++i)
    input.read (reinterpret_cast<char*>(mVertices[i].data()), 3 * sizeof(float));

  unsigned int nf;
  input.read (reinterpret_cast<char*>(&nf), sizeof(unsigned int));
  mFaces.resize (nf);
  for (size_t i = 0; i < nf; ++i)
    input.read (reinterpret_cast<char*>(mFaces[i].data()), 3 * sizeof(unsigned int));

  input.close();

  setVertexNormals();

  cLog::log (LOGINFO, fmt::format ("cModel::loadRawFile {} vertices:{} faces:{}",
                                   fileName, mVertices.size(), mFaces.size()));
  }
//}}}
//{{{
void cModel::loadObjFile (string const& fileName) {

    // Go through each loaded mesh and out its contents
    objLoader::cLoader loader;
    bool loadout = loader.loadFile (fileName);

    for (int i = 0; i < loader.LoadedMeshes.size(); i++) {
      // Copy one of the loaded meshes to be our current mesh
      objLoader::Mesh curMesh = loader.LoadedMeshes[i];

      // Print Mesh Name
      cout << "Mesh " << i << ": " << curMesh.MeshName << "\n";

      // Print Vertices
      cout << "Vertices:\n";

      // Go through each vertex and print its number,
      //  position, normal, and texture coordinate
      //for (int j = 0; j < curMesh.Vertices.size(); j++) {
      //  cout << "V" << j << ": " <<
      //    "P(" << curMesh.Vertices[j].Position.X << ", " << curMesh.Vertices[j].Position.Y << ", " << curMesh.Vertices[j].Position.Z << ") " <<
      //    "N(" << curMesh.Vertices[j].Normal.X << ", " << curMesh.Vertices[j].Normal.Y << ", " << curMesh.Vertices[j].Normal.Z << ") " <<
      //    "TC(" << curMesh.Vertices[j].TextureCoordinate.X << ", " << curMesh.Vertices[j].TextureCoordinate.Y << ")\n";
      //  }

      // Print Indices
      cout << "Indices:\n";

      // Go through every 3rd index and print the
      //  triangle that these indices represent
      //for (int j = 0; j < curMesh.Indices.size(); j += 3)
      //  cout << "T" << j / 3 << ": " << curMesh.Indices[j] << ", " << curMesh.Indices[j + 1] << ", " << curMesh.Indices[j + 2] << "\n";

      // Print Material
      cout << "Material: " << curMesh.MeshMaterial.name << "\n";
      cout << "Ambient Color: " << curMesh.MeshMaterial.Ka.X << ", " << curMesh.MeshMaterial.Ka.Y << ", " << curMesh.MeshMaterial.Ka.Z << "\n";
      cout << "Diffuse Color: " << curMesh.MeshMaterial.Kd.X << ", " << curMesh.MeshMaterial.Kd.Y << ", " << curMesh.MeshMaterial.Kd.Z << "\n";
      cout << "Specular Color: " << curMesh.MeshMaterial.Ks.X << ", " << curMesh.MeshMaterial.Ks.Y << ", " << curMesh.MeshMaterial.Ks.Z << "\n";
      cout << "Specular Exponent: " << curMesh.MeshMaterial.Ns << "\n";
      cout << "Optical Density: " << curMesh.MeshMaterial.Ni << "\n";
      cout << "Dissolve: " << curMesh.MeshMaterial.d << "\n";
      cout << "Illumination: " << curMesh.MeshMaterial.illum << "\n";
      cout << "Ambient Texture Map: " << curMesh.MeshMaterial.map_Ka << "\n";
      cout << "Diffuse Texture Map: " << curMesh.MeshMaterial.map_Kd << "\n";
      cout << "Specular Texture Map: " << curMesh.MeshMaterial.map_Ks << "\n";
      cout << "Alpha Texture Map: " << curMesh.MeshMaterial.map_d << "\n";
      cout << "Bump Map: " << curMesh.MeshMaterial.map_bump << "\n";

      // Leave a space to separate from the next mesh
      cout << "\n";
      }

  mVertices.resize (loader.LoadedVertices.size());
  for (size_t i = 0; i < loader.LoadedVertices.size(); ++i)
    mVertices.push_back (Eigen::Vector3f (loader.LoadedVertices[i].Position.X,
                                          loader.LoadedVertices[i].Position.Y,
                                          loader.LoadedVertices[i].Position.Z));

  //mFaces.resize (loader.LoadedMeshes.size());
  //for (size_t i = 0; i < loader.LoadedMeshes.size(); ++i)
  //  mFaces.push_back (loader.LoadedMeshes[i]);

  //setVertexNormals();

  cLog::log (LOGINFO, fmt::format ("cModel::LoadedMeshes {} LoadedMeshes {} LoadedVertices:{} LoadedIndices:{} LoadedMaterials:{}",
                                   fileName,
                                   loader.LoadedMeshes.size(), loader.LoadedVertices.size(),
                                   loader.LoadedIndices.size(), loader.LoadedMaterials.size()));
  }
//}}}
//{{{
void cModel::ripple() {

  mTime += 25.f / 1000.f;

  const float k = 50.0f;
  const float a = 0.03f;
  const float v = 10.0f;
  for (unsigned int i(0); i < mVertices.size(); ++i) {
    const float x = mRefVertices[i].x() + mRefVertices[i].y() + mRefVertices[i].z();
    const float u = 5.0f * (x - 0.75f * sin(2.5f * mTime));
    const float w = (a / 2.0f) * (1.0f + sin(k * x + v * mTime));
    mVertices[i] = mRefVertices[i] + (exp(-u * u) * w) * mRefNormals[i];
    }

  setVertexNormals();
  }
//}}}

// - private
//{{{
void cModel::setVertexNormals() {

  unsigned int nf(static_cast<unsigned int>(mFaces.size()));
  unsigned int nv(static_cast<unsigned int>(mVertices.size()));

  mNormals.resize (mVertices.size());
  fill (mNormals.begin(), mNormals.end(), Eigen::Vector3f::Zero());

  for (size_t i = 0; i < mFaces.size(); ++i) {
    array<unsigned int, 3> const& f_i = mFaces[i];

    Eigen::Vector3f const& p0(mVertices[f_i[0]]);
    Eigen::Vector3f const& p1(mVertices[f_i[1]]);
    Eigen::Vector3f const& p2(mVertices[f_i[2]]);

    Eigen::Vector3f n_i = (p0 - p1).cross(p0 - p2);

    mNormals[f_i[0]] += n_i;
    mNormals[f_i[1]] += n_i;
    mNormals[f_i[2]] += n_i;
    }

  for (size_t i = 0; i < mVertices.size(); ++i)
    if (!mNormals[i].isZero())
      mNormals[i].normalize();
  }
//}}}

// cSurfelModel
//{{{
void cSurfelModel::load (int modelIndex) {

  switch (modelIndex) {
    case 0:
      createModel ("../models/stanford_dragon_v40k_f80k.raw");
      break;

    case 1:
      createModel ("../models/stanford_dragon_v344k_f688k.raw");
      break;

    case 2:
      createChecker (100,100);
      break;

    case 3:
      createCube();
      break;

    case 4: {
      createPiccy (mFileName.empty() ? "../piccies/test.png" : mFileName);
      break;
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
        mModel[j].clipPlane= Eigen::Vector3f::Zero();

        float h = min((abs(p0.x()) / 0.45f) * 360.0f, 360.0f);
        float r, g, b;
        hsv2rgb (h, 1.0f, 1.0f, r, g, b);
        mModel[j].rgba = static_cast<uint32_t>(r * 255.0f)
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
void cSurfelModel::createModel (const string& filename) {

  try {
    loadRawFile (filename);
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
void cSurfelModel::createPiccy (const string& filename) {

  mVertices.clear();
  mFaces.clear();
  mNormals.clear();
  mRefVertices.clear();
  mRefNormals.clear();

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
