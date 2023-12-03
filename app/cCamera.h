#pragma once
#include <Eigen/Dense>
#include <memory>
#include "baseClasses.h"

//{{{
class cFrustum {
public:
  float& left() { return mLeft; }
  float& right() { return mRight; }

  float& bottom() { return mBottom; }
  float& top() { return mTop; }

  float& near_() { return mNear; }
  float& far_() { return mFar; }

private:
  float mLeft;
  float mRight;
  float mBottom;
  float mTop;
  float mNear;
  float mFar;
  };
//}}}
//{{{
class cTrackball {
public:
  Eigen::Quaternionf const& operator()(float u0_x, float u0_y, float u1_x, float u1_y);

private:
  float projectToSphere (float r, float x, float y) const;

  Eigen::Quaternionf rotation;
  };
//}}}
//{{{
class cCamera {
public:
  cCamera();
  virtual ~cCamera();

  cFrustum const& getFrustum() const;
  Eigen::Matrix4f const& getModelViewMatrix() const;
  Eigen::Matrix4f const& getProjectionMatrix() const;

  void setFrustum (cFrustum const& frustum);
  void setAspect (float aspect);
  void setPerspective (float fovy, float aspect, float near_, float far_);

  void setPosition (Eigen::Vector3f const& position);

  void setOrientation (Eigen::Matrix3f const& orientation);
  void setOrientation (Eigen::Quaternionf const& orientation);

  void rotate (Eigen::Quaternionf const& rotation);
  void rotate (Eigen::Matrix3f const& rotation);

  void translate (Eigen::Vector3f const& translation);

  void trackballBeginMotion (float begin_x, float begin_y);
  void trackballEndMotionRotate (float end_x, float end_y);
  void trackballEndMotionZoom (float end_x, float end_y);
  void trackballEndMotionTranslate (float end_x, float end_y);

private:
  void setProjectionMatrixFromFrustum();
  void setModelViewMatrixFromOrientation();

  // vars
  Eigen::Vector3f mPosition;
  Eigen::Quaternionf mOrientation;

  Eigen::Matrix4f mModelViewMatrix;
  Eigen::Matrix4f mProjectionMatrix;

  cFrustum mFrustum;
  float mFovyRad;
  float mAspect;

  float mBeginX;
  float mBeginY;

  cTrackball mTrackball;
  };
//}}}
//{{{
class cUniformCamera : public cUniform {
public:
  cUniformCamera();
  void set (cCamera const& camera);
  };
//}}}
