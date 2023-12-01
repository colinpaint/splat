#pragma once
#include "cRender.h"

//{{{
class cUniformBufferRaycast : public GLviz::glUniformBuffer {
public:
  cUniformBufferRaycast();

  void set_buffer_data (Eigen::Matrix4f const& projection_matrix_inv, GLint const* viewport);
  };
//}}}
//{{{
class cUniformBufferFrustum : public GLviz::glUniformBuffer {
public:
  cUniformBufferFrustum();

  void set_buffer_data (Eigen::Vector4f const* frustum_plane);
  };
//}}}
//{{{
class cUniformBufferParameter : public GLviz::glUniformBuffer {
public:
  cUniformBufferParameter();

  void set_buffer_data (Eigen::Vector3f const& color, float shine,
                        float radius_scale, float ewa_radius, float epsilon);
  };
//}}}

//{{{
class cProgramAttribute : public glProgram {
public:
  cProgramAttribute();

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
  bool mSmooth;
  bool m_color_material;
  unsigned int m_pointsize_method;
  };
//}}}
//{{{
class cProgramFinal : public glProgram {
public:
  cProgramFinal();

  void set_multisampling (bool enable);
  void set_smooth (bool enable);

private:
  void initShader();
  void initProgram();

  glVertexShader m_Final_vs_obj;
  glFragmentShader m_Final_fs_obj;
  glFragmentShader m_lighting_fs_obj;

  bool mSmooth;
  bool mMulitSample;
  };
//}}}
//{{{
class cFrameBuffer {
public:
  cFrameBuffer();
  ~cFrameBuffer();

  GLuint color_texture();

  void enable_depth_texture();
  void disable_depth_texture();
  GLuint depth_texture();

  void attachNormalTexture();
  void detachNormalTexture();
  GLuint normal_texture();

  void set_multisample (bool enable = true);

  void bind();
  void unbind();
  void resize (GLint width, GLint height);

private:
  void initialize();
  void removeDeleteAttachments();

  GLuint mFbo;
  GLuint mColor;
  GLuint mNormal;
  GLuint mDepth;

  struct sImpl;
  struct sDefault;
  struct sMultisample;

  std::unique_ptr<sImpl> mPimpl;
  };
//}}}

class cSplatRender : public cRender {
public:
  cSplatRender (GLviz::Camera const& camera);
  virtual ~cSplatRender();

  virtual void setBackFaceCull (bool enable = true) final;
  virtual void setMultiSample (bool enable = true) final;

  virtual void bindUniforms() final;

  virtual void gui() final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void display (cModel* model) final;
  virtual void resize (int width, int height) final;

private:
  //{{{  gui access
  bool smooth() const;
  void set_smooth (bool enable = true);

  // softZ
  bool soft_zbuffer() const { return mSoftZbuffer; }
  //{{{
  void set_soft_zbuffer (bool enable) {

    if (mSoftZbuffer != enable) {
      if (!enable) {
        mEwaFilter = false;
         mAttribute.set_ewa_filter(false);
         }

      mSoftZbuffer = enable;
      }
    }
  //}}}

  float getSoftZbufferEpsilon() const { return m_epsilon; }
  void setSoftZbufferEpsilon (float epsilon) { m_epsilon = epsilon; }

  // ewa
  bool ewa_filter() const { return mEwaFilter; }
  //{{{
  void set_ewa_filter (bool enable) {

    if (mSoftZbuffer && mEwaFilter != enable) {
      mEwaFilter = enable;
      mAttribute.set_ewa_filter(enable);
      }
    }
  //}}}

  float ewa_radius() const { return m_ewa_radius; }
  void set_ewa_radius (float ewa_radius) { m_ewa_radius = ewa_radius; }

  unsigned int pointsize_method() const;
  void set_pointsize_method (unsigned int pointsize_method);

  float radius_scale() const;
  void set_radius_scale (float radius_scale);
  //}}}

  void setupProgramObjects();
  void setupFilterKernel();
  void setupScreenQuad();
  void setupVertexArrayBuffer();
  void setupUniforms (glProgram& program);

  void renderPass (bool depth_only);

  //{{{  vars
  GLuint mVao;
  GLuint mVbo;
  size_t mNumSurfels;

  GLuint m_uv_vbo;
  GLuint m_rect_vao;
  GLuint m_rect_vertices_vbo;
  GLuint m_rect_texture_uv_vbo;

  GLuint mFilterKernel;

  cProgramAttribute mVisibility;
  cProgramAttribute mAttribute;
  cProgramFinal mFinal;

  cFrameBuffer mFrameBuffer;

  bool mSoftZbuffer;
  bool mBackFaceCull;
  bool mSmooth;
  bool mEwaFilter;

  unsigned int m_pointsize_method;
  float m_epsilon;
  float m_radius_scale;
  float m_ewa_radius;

  cUniformBufferRaycast m_uniform_raycast;
  cUniformBufferFrustum m_uniform_frustum;
  cUniformBufferParameter m_uniform_parameter;
  //}}}
  };
