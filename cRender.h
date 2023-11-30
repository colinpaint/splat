#pragma once
#include <memory>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <Eigen/Core>

#include "glviz/glviz.h"
#include "glviz/camera.h"
#include "glviz/program.h"

//{{{
class cSurfel {
public:
  cSurfel() {}
  cSurfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_, Eigen::Vector3f p_, unsigned int rgba_)
    : centre(c_), major(u_), minor(v_), clipPlane(p_), rgba(rgba_) {}

  // model
  Eigen::Vector3f centre;    // ellipse center point
  Eigen::Vector3f major;     // ellipse major axis
  Eigen::Vector3f minor;     // ellipse minor axis
  Eigen::Vector3f clipPlane; // clipping plane

  uint32_t rgba;             // color
  };
//}}}
//{{{
class cSurfels {
public:
  cSurfels() {}

  void* getArray() { return &mModel.front(); }
  size_t getSize() const { return mModel.size(); }

  void loadModel (int model);

private:
  void hsv2rgb (float h, float s, float v, float& r, float& g, float& b);
  void steinerCircumEllipse (float const* v0_ptr, float const* v1_ptr, float const* v2_ptr,
                             float* p0_ptr, float* t1_ptr, float* t2_ptr);
  void meshToSurfel (std::vector <Eigen::Vector3f> const& vertices,
                     std::vector <std::array <unsigned int, 3>> const& faces);
  void createModel (const std::string& filename);

  void createChecker (size_t width, size_t height);
  void createPiccy (const std::string& filename);
  void createCube();

  std::vector <cSurfel> mModel;
  };
//}}}

class cRender {
public:
  cRender (GLviz::Camera const& camera) : m_camera(camera) {}
  virtual ~cRender() {}

  bool getMultiSample() const { return mMultiSample; }
  bool getBackFaceCull() const { return mBackFaceCull; }

  bool getColorMaterial() const { return m_color_material; };
  void setColorMaterial (bool enable = true) { m_color_material = enable; }

  Eigen::Vector3f getMaterialColor() const { return mColor; }
  void setMaterialColor (Eigen::Vector3f const color) { mColor = color; }

  float getMaterialShininess() const { return mShininess; }
  void setMaterialShininess (float shininess) { mShininess = shininess; };

  // overides
  virtual void setMultiSample (bool enable = true) { mMultiSample = enable; }
  virtual void setBackFaceCull (bool enable = true) { mBackFaceCull = enable; }

  // abstract
  virtual bool keyboard (SDL_Keycode key) { return false; }
  virtual void resize (int width, int height) = 0;
  virtual void render (cSurfels& model) = 0;

protected:
  GLviz::Camera const& m_camera;
  GLviz::UniformBufferCamera m_uniform_camera;

  bool mMultiSample = false;
  bool mBackFaceCull = false;

  bool m_color_material = false;
  Eigen::Vector3f mColor;
  float mShininess = 1.f;
  };
