//{{{  includes
#include "cCamera.h"
#include <Eigen/Dense>

using namespace std;
//}}}

//{{{  cTrackball
// This size should really be based on the distance from the center of
// rotation to the point on the object underneath the mouse.  That
// point would then track the mouse as closely as possible.  This is a
// simple example, though, so that is left as an Exercise for the Programmer
const float kTrackballSize = 1.0f;
//{{{
Eigen::Quaternionf const& cTrackball::operator()(float u0_x, float u0_y, float u1_x, float u1_y) {
// Ok, simulate a track-ball.
// Project the points onto the virtual trackball, then figure out the axis of rotation,
// which is the cross product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
// This is a deformed trackball-- is a trackball in the center,
// - but is deformed into a hyperbolic sheet of rotation away from the center.
// This particular function was chosen after trying out several variations.
// It is assumed that the arguments to this routine are in the range (-1.0 ... 1.0)

  if ((u0_x == u1_x) && (u0_y == u1_y)) {
    // Zero rotation.
    rotation = Eigen::Quaternionf::Identity();
    return rotation;
    }

  // First, figure out z-coordinates for projection of P1 and P2 to
  // deformed sphere.
  Eigen::Vector3f u0(u0_x, u0_y, projectToSphere (kTrackballSize, u0_x, u0_y));
  Eigen::Vector3f u1(u1_x, u1_y, projectToSphere (kTrackballSize, u1_x, u1_y));

  // Calculate rotation axis.
  Eigen::Vector3f axis = u1.cross (u0);
  axis.normalize();

  // Calculate rotation angle.
  Eigen::Vector3f d = u0 - u1;

  // Clamp t to [-1.0, 1.0].
  float t = d.norm() / (2.0f * kTrackballSize);
  if (t > 1.0f)
    t = 1.0f;
  else if (t < -1.0f)
    t = -1.0f;

  rotation = Eigen::AngleAxisf (2.0f * asin(t), axis);
  return rotation;
  }
//}}}
//{{{
inline float cTrackball::projectToSphere (float r, float x, float y) const {
// Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
// if we are away from the center of the sphere.

  const float sqrt2_div2 = 0.70710678118654752440f;
  float d = sqrt (x * x + y * y);

  float z;
  if (d < r * sqrt2_div2) // Inside sphere
    z = sqrt (r * r - d * d);
  else // On hyperbola
    z = r * r / (2.0f * d);

  return z;
  }
//}}}
//}}}

// cCamera
//{{{
cCamera::cCamera() : mPosition(Eigen::Vector3f::Zero()),
                     mOrientation(Eigen::Quaternionf::Identity()) {

  setPerspective (60.0f, 4.0f / 3.0f, 0.25f, 10.0f);
  setModelViewMatrixFromOrientation();
  }
//}}}
cCamera::~cCamera() { }

cFrustum const& cCamera::getFrustum() const { return mFrustum; }
Eigen::Matrix4f const& cCamera::getModelViewMatrix() const { return mModelViewMatrix; }
Eigen::Matrix4f const& cCamera::getProjectionMatrix() const { return mProjectionMatrix; }

//{{{
void cCamera::setProjectionMatrixFromFrustum() {

  const float l = mFrustum.left();
  const float r = mFrustum.right();

  const float b = mFrustum.bottom();
  const float t = mFrustum.top();

  const float n = mFrustum.near_();
  const float f = mFrustum.far_();

  mProjectionMatrix <<
      (2.0f * n) / (r - l), 0.0f, (r + l) / (r - l), 0.0f,
      0.0f, (2.0f * n) / (t - b), (t + b) / (t - b), 0.0f,
      0.0f, 0.0f, -(f + n) / (f - n), -(2.0f * n * f) / (f - n),
      0.0f, 0.0f, -1.0f, 0.0f;
  }
//}}}
//{{{
void cCamera::setModelViewMatrixFromOrientation() {

  Eigen::Matrix3f dir = Eigen::AngleAxisf (mOrientation).inverse().toRotationMatrix();

  mModelViewMatrix = Eigen::Matrix4f::Identity();

  Eigen::Vector3f ori = dir * mPosition;

  // Translation * Rotation
  mModelViewMatrix.topLeftCorner (3, 3) = dir;
  mModelViewMatrix.topRightCorner (3, 1) = mPosition;
  }
//}}}

//{{{
void cCamera::setFrustum (cFrustum const& frustum) {

  mFrustum = frustum;

  mFovyRad = 2.0f * std::atan(mFrustum.top() / mFrustum.near_());
  mAspect = mFrustum.right() / mFrustum.top();

  setProjectionMatrixFromFrustum();
  }
//}}}
//{{{
void cCamera::setPerspective (float fovy, float aspect, float near_, float far_) {

  float const pi = 3.14159265358979323846f;

  mFovyRad = pi / 180.0f * fovy;
  mAspect = aspect;

  mFrustum.top() = std::tan(mFovyRad / 2.0f) * near_;
  mFrustum.bottom() = -mFrustum.top();
  mFrustum.right() = mFrustum.top() * mAspect;
  mFrustum.left() = -mFrustum.right();
  mFrustum.near_() = near_;
  mFrustum.far_() = far_;

  setProjectionMatrixFromFrustum();
  }
//}}}
//{{{
void cCamera::setAspect (float aspect) {

  mAspect = aspect;

  mFrustum.right() = mFrustum.top() * mAspect;
  mFrustum.left() = -mFrustum.right();

  setProjectionMatrixFromFrustum();
  }
//}}}
//{{{
void cCamera::setPosition (Eigen::Vector3f const& position) {

  mPosition = position;
  setModelViewMatrixFromOrientation();
  }
//}}}
//{{{
void cCamera::setOrientation (Eigen::Matrix3f const& orientation) {

  mOrientation = Eigen::Quaternionf(orientation);
  mOrientation.normalize();

  setModelViewMatrixFromOrientation();
  }
//}}}
//{{{
void cCamera::setOrientation (Eigen::Quaternionf const& orientation) {

  mOrientation = orientation;
  mOrientation.normalize();

  setModelViewMatrixFromOrientation();
  }
//}}}

//{{{
void cCamera::rotate (Eigen::Quaternionf const& rotation) {

  Eigen::Quaternionf ret = mOrientation * rotation;
  mOrientation = ret;
  mOrientation.normalize();

  setModelViewMatrixFromOrientation();
  }
//}}}
void cCamera::rotate (Eigen::Matrix3f const& rotation) { rotate(Eigen::Quaternionf(rotation)); }

//{{{
void cCamera::translate (Eigen::Vector3f const& translation) {

  mPosition += translation;
  setModelViewMatrixFromOrientation();
  }
//}}}

//{{{
void cCamera::trackballBeginMotion (float begin_x, float begin_y) {
  mBeginX = begin_x;
  mBeginY = begin_y;
  }
//}}}
//{{{
void cCamera::trackballEndMotionRotate (float end_x, float end_y) {

  float u0_x = 2.0f * mBeginX - 1.0f;
  float u0_y = 1.0f - 2.0f * mBeginY;

  float u1_x = 2.0f * end_x - 1.0f;
  float u1_y = 1.0f - 2.0f * end_y;

  rotate (mTrackball (u0_x, u0_y, u1_x, u1_y));

  trackballBeginMotion (end_x, end_y);
  }
//}}}
//{{{
void cCamera::trackballEndMotionZoom (float end_x, float end_y) {

  float dy = end_y - mBeginY;
  translate (Eigen::Vector3f(0.0f, 0.0f, 2.0f * dy));

  trackballBeginMotion (end_x, end_y);
  }
//}}}
//{{{
void cCamera::trackballEndMotionTranslate (float end_x, float end_y) {

  float dx = end_x - mBeginX;
  float dy = end_y - mBeginY;
  translate (Eigen::Vector3f(2.0f * dx, -2.0f * dy, 0.0f));

  trackballBeginMotion (end_x, end_y);
  }
//}}}

// cUniformCamera
cUniformCamera::cUniformCamera() : cUniform(48 * sizeof(GLfloat)) { }
//{{{
void cUniformCamera::set (cCamera const& camera) {

  Eigen::Matrix4f const& modelView_matrix = camera.getModelViewMatrix();
  Eigen::Matrix4f modelView_matrix_it = modelView_matrix.inverse().transpose();
  Eigen::Matrix4f const& projection_matrix = camera.getProjectionMatrix();

  bind();
  glBufferSubData (GL_UNIFORM_BUFFER, 0, 16 * sizeof(GLfloat), modelView_matrix.data());
  glBufferSubData (GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), 16 * sizeof(GLfloat), modelView_matrix_it.data());
  glBufferSubData (GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), 16 * sizeof(GLfloat), projection_matrix.data());
  unbind();
  }
//}}}
