#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <Eigen/Core>
#include "glviz/camera.h"
#include "glviz/program.h"

//{{{
struct sSurfel {
  sSurfel() {}
  sSurfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_, Eigen::Vector3f p_, unsigned int rgba_)
    : centre(c_), major(u_), minor(v_), clipPlane(p_), rgba(rgba_) {}

  Eigen::Vector3f centre;    // ellipse center point
  Eigen::Vector3f major;     // ellipse major axis
  Eigen::Vector3f minor;     // ellipse minor axis
  Eigen::Vector3f clipPlane; // clipping plane

  uint32_t rgba;             // color
  };
//}}}

class cRender {
public:
  cRender (GLviz::Camera const& camera) : m_camera(camera) {}
  virtual ~cRender() {}

  bool getMultiSample() const { return mMultiSample; }
  virtual void setMultiSample (bool enable = true) { mMultiSample = enable; }

  bool getBackFaceCull() const { return mBackFaceCull; }
  void setBackFaceCull (bool enable = true) { mBackFaceCull = enable; }

  bool getColorMaterial() const { return m_color_material; };
  void setColorMaterial (bool enable = true) { m_color_material = enable; }

  Eigen::Vector3f getMaterialColor() const { return mColor; }
  void setMaterialColor (Eigen::Vector3f const color) { mColor = color; }

  float getMaterialShininess() const { return mShininess; }
  void setMaterialShininess (float shininess) { mShininess = shininess; };

  // abstract
  virtual void resize (int width, int height) = 0;
  virtual void render (std::vector<sSurfel> const& visible_geometry) = 0;

protected:
  GLviz::Camera const& m_camera;
  GLviz::UniformBufferCamera m_uniform_camera;

  bool mMultiSample = false;
  bool mBackFaceCull = false;

  bool m_color_material = false;
  Eigen::Vector3f mColor;
  float mShininess = 1.f;
  };
