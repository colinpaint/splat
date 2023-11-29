#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <Eigen/Core>
#include "glviz/shader.h"
#include "glviz/program.h"
#include "glviz/buffer.h"
#include "glviz/camera.h"

//{{{
struct sSurfel {
  sSurfel() {}
  sSurfel(Eigen::Vector3f c_, Eigen::Vector3f u_, Eigen::Vector3f v_, Eigen::Vector3f p_, unsigned int rgba_)
    : centre(c_), major(u_), minor(v_), clipPlane(p_), rgba(rgba_) {}

  Eigen::Vector3f centre;    // ellipse center point
  Eigen::Vector3f major;     // ellipse major axis
  Eigen::Vector3f minor;     // ellipse minor axis
  Eigen::Vector3f clipPlane; // clipping plane

  uint32_t rgba;             // color
  };
//}}}

//{{{
class UniformBufferRaycast : public GLviz::glUniformBuffer {
public:
  UniformBufferRaycast();

  void set_buffer_data (Eigen::Matrix4f const& projection_matrix_inv, GLint const* viewport);
  };
//}}}
//{{{
class UniformBufferFrustum : public GLviz::glUniformBuffer {
public:
  UniformBufferFrustum();

  void set_buffer_data (Eigen::Vector4f const* frustum_plane);
  };
//}}}
//{{{
class UniformBufferParameter : public GLviz::glUniformBuffer {
public:
  UniformBufferParameter();

  void set_buffer_data (Eigen::Vector3f const& color, float shininess,
                        float radius_scale, float ewa_radius, float epsilon);
  };
//}}}

//{{{
class ProgramAttribute : public glProgram {
public:
  ProgramAttribute();

  void set_ewa_filter (bool enable = true);
  void set_pointsize_method (unsigned int pointsize_method);
  void set_backface_culling (bool enable = true);
  void set_visibility_pass (bool enable = true);
  void set_smooth (bool enable = true);
  void set_color_material (bool enable = true);

private:
  void initShader();
  void initProgram();

  glVertexShader mAttributeVs;
  glVertexShader mLightingVs;
  glFragmentShader mAttributeFs;

  bool m_ewa_filter;
  bool m_backface_culling;
  bool m_visibility_pass;
  bool m_smooth;
  bool m_color_material;
  unsigned int m_pointsize_method;
  };
//}}}
//{{{
class ProgramFinal : public glProgram {
public:
  ProgramFinal();

  void set_multisampling (bool enable);
  void set_smooth (bool enable);

private:
  void initShader();
  void initProgram();

  glVertexShader m_Final_vs_obj;
  glFragmentShader m_Final_fs_obj;
  glFragmentShader m_lighting_fs_obj;

  bool m_smooth;
  bool m_multisampling;
  };
//}}}
//{{{
class Framebuffer {
public:
  Framebuffer();
  ~Framebuffer();

  GLuint color_texture();

  void enable_depth_texture();
  void disable_depth_texture();
  GLuint depth_texture();

  void attach_normal_texture();
  void detach_normal_texture();
  GLuint normal_texture();

  void set_multisample (bool enable = true);

  void bind();
  void unbind();
  void resize (GLint width, GLint height);

private:
  void initialize();
  void remove_and_delete_attachments();

  GLuint m_fbo;
  GLuint m_color;
  GLuint m_normal;
  GLuint m_depth;

  struct Impl;
  struct Default;
  struct Multisample;

  std::unique_ptr<Impl> m_pimpl;
  };
//}}}

//{{{
class cSplatRender {
public:
  cSplatRender (GLviz::Camera const& camera);
  virtual ~cSplatRender();

  bool smooth() const;
  void set_smooth (bool enable = true);

  bool color_material() const;
  void set_color_material (bool enable = true);

  bool backface_culling() const;
  void set_backface_culling (bool enable = true);

  bool soft_zbuffer() const;
  void set_soft_zbuffer (bool enable = true);

  float soft_zbuffer_epsilon() const;
  void set_soft_zbuffer_epsilon (float epsilon);

  unsigned int pointsize_method() const;
  void set_pointsize_method (unsigned int pointsize_method);

  bool ewa_filter() const;
  void set_ewa_filter (bool enable = true);

  bool multisample() const;
  void set_multisample (bool enable = true);

  float const* material_color() const;
  void set_material_color (float const* color_ptr);
  float material_shininess() const;
  void set_material_shininess (float shininess);

  float radius_scale() const;
  void set_radius_scale (float radius_scale);

  float ewa_radius() const;
  void set_ewa_radius (float ewa_radius);

  void resize (int width, int height);
  void render (std::vector<sSurfel> const& visible_geometry);

private:
  void setup_program_objects();
  void setup_filter_kernel();
  void setup_screen_size_quad();
  void setup_vertex_array_buffer_object();
  void setupUniforms (glProgram& program);

  void beginFrame();
  void endFrame();
  void renderPass (bool depth_only = false);

  //{{{  vars
  GLviz::Camera const& m_camera;
  GLviz::UniformBufferCamera m_uniform_camera;

  GLuint m_vbo;
  GLuint m_vao;
  unsigned int m_num_pts;

  GLuint m_uv_vbo;
  GLuint m_rect_vao;
  GLuint m_rect_vertices_vbo;
  GLuint m_rect_texture_uv_vbo;
  GLuint m_filter_kernel;

  ProgramAttribute m_visibility;
  ProgramAttribute m_attribute;
  ProgramFinal m_Final;

  Framebuffer m_fbo;

  bool m_soft_zbuffer;
  bool m_backface_culling;
  bool m_smooth;
  bool m_color_material;
  bool m_ewa_filter;
  bool m_multisample;

  unsigned int m_pointsize_method;
  Eigen::Vector3f m_color;
  float m_epsilon;
  float m_shininess;
  float m_radius_scale;
  float m_ewa_radius;

  UniformBufferRaycast m_uniform_raycast;
  UniformBufferFrustum m_uniform_frustum;
  UniformBufferParameter m_uniform_parameter;
  //}}}
  };
//}}}
