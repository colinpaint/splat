#pragma once
#include <Eigen/Dense>
#include <memory>

class Trackball;

namespace GLviz {
  //{{{
  class Frustum
  {

  public:
      float& left() { return m_left; }
      float& right() { return m_right; }

      float& bottom() { return m_bottom; }
      float& top() { return m_top; }

      float& near_() { return m_near; }
      float& far_() { return m_far; }

  private:
      float m_left, m_right;
      float m_bottom, m_top;
      float m_near, m_far;
  };
  //}}}
  //{{{
  class Camera {
  public:
      Camera();
      virtual ~Camera();

      Eigen::Matrix4f const& get_modelview_matrix() const;
      Eigen::Matrix4f const& get_projection_matrix() const;
      Frustum const& get_frustum() const;

      void set_frustum(Frustum const& frustum);
      void set_aspect(float aspect);
      void set_perspective(float fovy, float aspect, float near_,
          float far_);

      void set_position(Eigen::Vector3f const& position);

      void set_orientation(Eigen::Matrix3f const& orientation);
      void set_orientation(Eigen::Quaternionf const& orientation);

      void rotate(Eigen::Quaternionf const& rotation);
      void rotate(Eigen::Matrix3f const& rotation);

      void translate(Eigen::Vector3f const& translation);

      void trackball_begin_motion(float begin_x, float begin_y);

      void trackball_end_motion_rotate(float end_x, float end_y);
      void trackball_end_motion_zoom(float end_x, float end_y);
      void trackball_end_motion_translate(float end_x, float end_y);

  private:
      void set_projection_matrix_from_frustum();
      void set_modelview_matrix_from_orientation();

  private:
      Eigen::Vector3f m_position;
      Eigen::Quaternionf m_orientation;

      Eigen::Matrix4f m_modelview_matrix;
      Eigen::Matrix4f m_projection_matrix;

      Frustum m_frustum;
      float m_fovy_rad;
      float m_aspect;

      float m_begin_x, m_begin_y;

      std::unique_ptr<Trackball> m_trackball;
  };
  //}}}
  }
