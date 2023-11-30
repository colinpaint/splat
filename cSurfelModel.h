#pragma once
#include <memory>
#include <string>
#include <array>
#include <vector>
#include <map>
#include <Eigen/Core>

#include "cModel.h"

class cSurfelModel : public cModel {
public:
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
  cSurfelModel() {}
  virtual ~cSurfelModel() = default;

  void* getArray() { return &mModel.front(); }
  size_t getSize() const { return mModel.size(); }

  virtual void load (int modelIndex);

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
