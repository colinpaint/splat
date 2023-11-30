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

class cSurfelModel;

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
  virtual void render (cSurfelModel& model) = 0;

protected:
  GLviz::Camera const& m_camera;
  GLviz::UniformBufferCamera m_uniform_camera;

  bool mMultiSample = false;
  bool mBackFaceCull = false;

  bool m_color_material = false;
  Eigen::Vector3f mColor;
  float mShininess = 1.f;
  };
