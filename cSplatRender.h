#pragma once
#include "cRender.h"

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

class cSplatRender : public cRender {
public:
  cSplatRender (GLviz::Camera const& camera);
  virtual ~cSplatRender();

  virtual void setBackFaceCull (bool enable = true) final;
  virtual void setMultiSample (bool enable = true) final;

  virtual void resize (int width, int height) final;
  virtual void render (cModel* model) final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void gui() final;

private:
  //{{{  gui access
  bool smooth() const;
  void set_smooth (bool enable = true);

  bool soft_zbuffer() const;
  void set_soft_zbuffer (bool enable = true);

  float getSoftZbufferEpsilon() const;
  void setSoftZbufferEpsilon (float epsilon);

  unsigned int pointsize_method() const;
  void set_pointsize_method (unsigned int pointsize_method);

  bool ewa_filter() const;
  void set_ewa_filter (bool enable = true);

  float ewa_radius() const;
  void set_ewa_radius (float ewa_radius);

  float radius_scale() const;
  void set_radius_scale (float radius_scale);
  //}}}

  void setupProgramObjects();
  void setup_filter_kernel();
  void setup_screen_size_quad();
  void setup_vertex_array_buffer_object();
  void setupUniforms (glProgram& program);

  void renderPass (bool depth_only);

  //{{{  vars
  GLuint m_vbo;
  GLuint m_vao;
  size_t mNumSurfels;

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
  bool m_ewa_filter;

  unsigned int m_pointsize_method;
  float m_epsilon;
  float m_radius_scale;
  float m_ewa_radius;

  UniformBufferRaycast m_uniform_raycast;
  UniformBufferFrustum m_uniform_frustum;
  UniformBufferParameter m_uniform_parameter;
  //}}}
  };
