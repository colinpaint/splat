#pragma once
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

  virtual void loadIndex (int modelIndex) final;
  virtual void loadObjFile (const std::string& fileName);
  virtual void loadRawFile (const std::string& fileName);
  void loadPiccyFile (const std::string& fileName);

  void* getArray() { return &mModel.front(); }
  size_t getSize() const { return mModel.size(); }

private:
  void hsv2rgb (float h, float s, float v, float& r, float& g, float& b);
  void steinerCircumEllipse (float const* v0_ptr, float const* v1_ptr, float const* v2_ptr,
                             float* p0_ptr, float* t1_ptr, float* t2_ptr);
  void meshToSurfel();
  void createModel (const std::string& fileName);

  void createChecker (size_t width, size_t height);
  void createPiccy (const std::string& fileName);
  void createCube();

  std::vector <cSurfel> mModel;
  };
