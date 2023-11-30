#pragma once
#include <array>
#include <vector>
#include <string>
#include <Eigen/Core>

class cMeshModel {
public:
  cMeshModel() {}

  void load (std::string const& filename);

  void setVertexNormals();

  std::vector <Eigen::Vector3f> mVertices;
  std::vector <Eigen::Vector3f> mNormals;

  std::vector <Eigen::Vector3f> mRefVertices;
  std::vector <Eigen::Vector3f> mRefNormals;

  std::vector <std::array <unsigned int,3>> mFaces;
  };
