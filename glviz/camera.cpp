//{{{  includes
#include "camera.h"
#include "trackball.h"
//}}}

namespace GLviz {
  //{{{
  Camera::Camera()
      : m_position(Eigen::Vector3f::Zero()),
        m_orientation(Eigen::Quaternionf::Identity()),
        m_trackball(new Trackball())
  {
      set_perspective(60.0f, 4.0f / 3.0f, 0.25f, 10.0f);
      set_modelview_matrix_from_orientation();
  }
  //}}}
  //{{{
  Camera::~Camera()
  {
  }
  //}}}

  Frustum const& Camera::get_frustum() const { return m_frustum; }
  Eigen::Matrix4f const& Camera::get_modelview_matrix() const { return m_modelview_matrix; }
  Eigen::Matrix4f const& Camera::get_projection_matrix() const { return m_projection_matrix; }

  //{{{
  void Camera::set_projection_matrix_from_frustum() {

    const float l = m_frustum.left();
    const float r = m_frustum.right();

    const float b = m_frustum.bottom();
    const float t = m_frustum.top();

    const float n = m_frustum.near_();
    const float f = m_frustum.far_();

    m_projection_matrix <<
        (2.0f * n) / (r - l), 0.0f, (r + l) / (r - l), 0.0f,
        0.0f, (2.0f * n) / (t - b), (t + b) / (t - b), 0.0f,
        0.0f, 0.0f, -(f + n) / (f - n), -(2.0f * n * f) / (f - n),
        0.0f, 0.0f, -1.0f, 0.0f;
    }
  //}}}
  //{{{
  void Camera::set_modelview_matrix_from_orientation() {

    Eigen::Matrix3f dir = Eigen::AngleAxisf (m_orientation).inverse().toRotationMatrix();

    m_modelview_matrix = Eigen::Matrix4f::Identity();

    Eigen::Vector3f ori = dir * m_position;

    // Translation * Rotation
    m_modelview_matrix.topLeftCorner(3, 3) = dir;
    m_modelview_matrix.topRightCorner(3, 1) = m_position;
    }
  //}}}

  //{{{
  void Camera::set_frustum (Frustum const& frustum) {

    m_frustum = frustum;

    m_fovy_rad = 2.0f * std::atan(m_frustum.top() / m_frustum.near_());
    m_aspect = m_frustum.right() / m_frustum.top();

    set_projection_matrix_from_frustum();
    }
  //}}}
  //{{{
  void Camera::set_perspective (float fovy, float aspect, float near_, float far_) {

    float const pi = 3.14159265358979323846f;

    m_fovy_rad = pi / 180.0f * fovy;
    m_aspect = aspect;

    m_frustum.top() = std::tan(m_fovy_rad / 2.0f) * near_;
    m_frustum.bottom() = -m_frustum.top();
    m_frustum.right() = m_frustum.top() * m_aspect;
    m_frustum.left() = -m_frustum.right();
    m_frustum.near_() = near_;
    m_frustum.far_() = far_;

    set_projection_matrix_from_frustum();
    }
  //}}}
  //{{{
  void Camera::set_aspect (float aspect) {

    m_aspect = aspect;

    m_frustum.right() = m_frustum.top() * m_aspect;
    m_frustum.left() = -m_frustum.right();

    set_projection_matrix_from_frustum();
    }
  //}}}
  //{{{
  void Camera::set_position (Eigen::Vector3f const& position) {

    m_position = position;
    set_modelview_matrix_from_orientation();
    }
  //}}}
  //{{{
  void Camera::set_orientation (Eigen::Matrix3f const& orientation) {

    m_orientation = Eigen::Quaternionf(orientation);
    m_orientation.normalize();

    set_modelview_matrix_from_orientation();
    }
  //}}}
  //{{{
  void Camera::set_orientation (Eigen::Quaternionf const& orientation) {

    m_orientation = orientation;
    m_orientation.normalize();

    set_modelview_matrix_from_orientation();
    }
  //}}}

  //{{{
  void Camera::rotate (Eigen::Quaternionf const& rotation) {

    Eigen::Quaternionf ret = m_orientation * rotation;
    m_orientation = ret;
    m_orientation.normalize();

    set_modelview_matrix_from_orientation();
    }
  //}}}
  void Camera::rotate (Eigen::Matrix3f const& rotation) { rotate(Eigen::Quaternionf(rotation)); }

  //{{{
  void Camera::translate (Eigen::Vector3f const& translation) {

    m_position += translation;
    set_modelview_matrix_from_orientation();
    }
  //}}}

  //{{{
  void Camera::trackball_begin_motion (float begin_x, float begin_y) {
    m_begin_x = begin_x;
    m_begin_y = begin_y;
    }
  //}}}
  //{{{
  void Camera::trackball_end_motion_rotate (float end_x, float end_y) {

    float u0_x = 2.0f * m_begin_x - 1.0f;
    float u0_y = 1.0f - 2.0f * m_begin_y;

    float u1_x = 2.0f * end_x - 1.0f;
    float u1_y = 1.0f - 2.0f * end_y;

    auto& trackball = *m_trackball;
    rotate (trackball(u0_x, u0_y, u1_x, u1_y));

    trackball_begin_motion(end_x, end_y);
    }
  //}}}
  //{{{
  void Camera::trackball_end_motion_zoom (float end_x, float end_y) {

    float dy = end_y - m_begin_y;
    translate (Eigen::Vector3f(0.0f, 0.0f, 2.0f * dy));
    trackball_begin_motion (end_x, end_y);
    }
  //}}}
  //{{{
  void Camera::trackball_end_motion_translate (float end_x, float end_y) {

    float dx = end_x - m_begin_x;
    float dy = end_y - m_begin_y;
    translate(Eigen::Vector3f(2.0f * dx, -2.0f * dy, 0.0f));

    trackball_begin_motion(end_x, end_y);
    }
  //}}}
  }
