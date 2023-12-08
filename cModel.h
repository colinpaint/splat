#pragma once
#include <memory>
#include <string>
#include <array>
#include <vector>
#include <map>

#include <Eigen/Core>

class cModel {
public:
  cModel() {}
  virtual ~cModel() = default;

  virtual void loadIndex (int index);
  virtual void loadObjFile (const std::string& fileName);
  virtual void loadRawFile (const std::string& fileName);

  size_t isSelectable() const { return mSelectable; }

  size_t getNumVertices() const { return mVertices.size(); }
  size_t getNumNormals() const { return mNormals.size(); }
  size_t getNumFaces() const { return mFaces.size(); }
  float* getScale() const { mScale; }
  Eigen::Vector3f getCentre() const { mCentre; }

  float* getVerticesData() { return (float*)mVertices.front().data(); }
  float* getNormalsData() { return (float*)mNormals.front().data(); }
  float* getFacesData() { return (float*)mFaces.front().data(); }

  void ripple();

  // vertices, normal, faces
  std::vector <Eigen::Vector3f> mVertices;
  std::vector <Eigen::Vector3f> mNormals;
  std::vector <std::array <unsigned int,3>> mFaces;

  // reference vertices, normals for ripple
  std::vector <Eigen::Vector3f> mRefVertices;
  std::vector <Eigen::Vector3f> mRefNormals;
  float mTime = 0.f;

  bool mSelectable = true;

private:
  void normaliseVertices();
  void setVertexNormals();

  float mScale = 1.f;
  Eigen::Vector3f mCentre;
  };
