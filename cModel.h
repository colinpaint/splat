#pragma once
#include <array>
#include <vector>
#include <string>
#include <Eigen/Core>

class cModel {
public:
  cModel() {}
  virtual ~cModel() = default;

  virtual void load (int modelIndex);
  void load (std::string const& filename);

  void setVertexNormals();
  void ripple();

  std::vector <Eigen::Vector3f> mVertices;
  std::vector <Eigen::Vector3f> mNormals;

  std::vector <Eigen::Vector3f> mRefVertices;
  std::vector <Eigen::Vector3f> mRefNormals;

  std::vector <std::array <unsigned int,3>> mFaces;

  float mTime = 0.f;
  };
