#pragma once
#include "cRender.h"
#include "glviz/shader.h"
#include "glviz/buffer.h"

//{{{
class cUniformRaycast : public GLviz::glUniformBuffer {
public:
  cUniformRaycast();
  void setBuffer (Eigen::Matrix4f const& projection_matrix_inv, GLint const* viewport);
  };
//}}}
//{{{
class cUniformFrustum : public GLviz::glUniformBuffer {
public:
  cUniformFrustum();
  void setBuffer (Eigen::Vector4f const* frustum_plane);
  };
//}}}
//{{{
class cUniformParameter : public GLviz::glUniformBuffer {
public:
  cUniformParameter();
  void setBuffer (Eigen::Vector3f const& color, float shine,
                  float radius_scale, float ewa_radius, float epsilon);
  };
//}}}

//{{{
class cProgramAttribute : public glProgram {
public:
  cProgramAttribute();

  void setBackFaceCull (bool enable = true);
  void setSmooth (bool enable = true);
  void setVisibilityPass (bool enable = true);
  void setColorMaterial (bool enable = true);
  void setEwaFilter (bool enable = true);
  void setPointSizeType (unsigned int pointSizeType);

private:
  void initShader();
  void initProgram();

  glVertexShader mAttributeVs;
  glVertexShader mLightVs;
  glFragmentShader mAttributeFs;

  bool mBackFaceCull = false;
  bool mSmooth = false;
  bool mVisibilityPass = true;
  bool mColorMaterial = false;
  bool mEwaFilter = false;
  unsigned int mPointSizeType = 0;
  };
//}}}
//{{{
class cProgramFinal : public glProgram {
public:
  cProgramFinal();

  void setMultiSample (bool enable);
  void setSmooth (bool enable);

private:
  void initShader();
  void initProgram();

  glVertexShader mFinalVs;
  glFragmentShader mFinalFs;
  glFragmentShader mLightFs;

  bool mMulitSample = false;
  bool mSmooth = false;;
  };
//}}}
//{{{
class cFrameBuffer {
public:
  cFrameBuffer();
  ~cFrameBuffer();

  GLuint getColorTexture();
  GLuint getDepthTexture();
  GLuint getNormalTexture();

  void enableDepthTexture();
  void disableDepthTexture();

  void attachNormalTexture();
  void detachNormalTexture();

  void setMultiSample (bool enable = true);

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

  virtual void use (bool multiSample, bool backFaceCull) final;

  virtual void gui() final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void display (cModel* model) final;
  virtual void resize (int width, int height) final;

private:
  bool getSmooth() const { return mSmooth; }
  void setSmooth (bool enable = true);
  //{{{  softZ
  bool getSoftZbuffer() const { return mSoftZbuffer; }
  void setSoftZbuffer (bool enable);

  float getSoftZbufferEpsilon() const { return mEpsilon; }
  void setSoftZbufferEpsilon (float epsilon) { mEpsilon = epsilon; }
  //}}}
  //{{{  ewa
  bool getEwaFilter() const { return mEwaFilter; }
  void setEwaFilter (bool enable);

  float getEwaRadius() const { return mEwaRadius; }
  void setEwaRadius (float ewaRadius) { mEwaRadius = ewaRadius; }
  //}}}
  unsigned int getPointSizeType() const { return mPointSizeType; }
  void setPointSizeType (unsigned int pointSizeType);

  float getRadiusScale() const { return mRadiusScale; }
  void setRadiusScale (float radiusScale) { mRadiusScale = radiusScale; }

  void setupProgramObjects();
  void setupFilterKernel();
  void setupVertexArrayBuffer();
  void setupScreenQuad();
  void setupUniforms (glProgram& program);

  void renderPass (bool depth_only);

  //{{{  vars
  GLuint mVao;
  GLuint mVbo;
  size_t mNumSurfels;

  GLuint muvVbo;
  GLuint mQuadVao;
  GLuint mQuadVerticesVbo;
  GLuint mQuadTextureVbo;

  GLuint mFilterKernel;

  cUniformRaycast mUniformRaycast;
  cUniformFrustum mUniformFrustum;
  cUniformParameter mUniformParameter;

  cProgramAttribute mVisibility;
  cProgramAttribute mAttribute;
  cProgramFinal mFinal;

  cFrameBuffer mFrameBuffer;

  //
  bool mSmooth;

  bool mSoftZbuffer;
  float mEpsilon;

  bool mEwaFilter;
  float mEwaRadius;

  unsigned int mPointSizeType;
  float mRadiusScale;
  //}}}
  };
