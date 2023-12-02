#pragma once
#include "cRender.h"
#include "glviz/shader.h"
#include "glviz/buffer.h"

//{{{
class cUniformRaycast : public GLviz::cUniformBuffer {
public:
  cUniformRaycast();
  void set (Eigen::Matrix4f const& projection_matrix_inv, GLint const* viewport);
  };
//}}}
//{{{
class cUniformFrustum : public GLviz::cUniformBuffer {
public:
  cUniformFrustum();
  void set (Eigen::Vector4f const* frustum_plane);
  };
//}}}
//{{{
class cUniformParameter : public GLviz::cUniformBuffer {
public:
  cUniformParameter();
  void set (Eigen::Vector3f const& color, float shine,
            float radius_scale, float ewa_radius, float epsilon);
  };
//}}}

//{{{
class cProgramAttribute : public cProgram {
public:
  cProgramAttribute();

  void setBackFaceCull (bool enable);
  void setSmooth (bool enable);
  void setVisibilityPass (bool enable);
  void setColorMaterial (bool enable);
  void setEwaFilter (bool enable);
  void setPointSizeType (unsigned int pointSizeType);

private:
  void initShader();
  void initProgram();

  cVertexShader mAttributeVs;
  cVertexShader mLightVs;
  cFragmentShader mAttributeFs;

  bool mBackFaceCull = false;
  bool mSmooth = false;
  bool mVisibilityPass = true;
  bool mColorMaterial = false;
  bool mEwaFilter = false;
  unsigned int mPointSizeType = 0;
  };
//}}}
//{{{
class cProgramFinal : public cProgram {
public:
  cProgramFinal();

  void setMultiSample (bool enable);
  void setSmooth (bool enable);

private:
  void initShader();
  void initProgram();

  cVertexShader mFinalVs;
  cFragmentShader mFinalFs;
  cFragmentShader mLightFs;

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
  cSplatRender (GLviz::cCamera const& camera);
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
  void setSmooth (bool enable);

  bool getSoftZbuffer() const { return mSoftZbuffer; }
  void setSoftZbuffer (bool enable);

  float getSoftZbufferEpsilon() const { return mEpsilon; }
  void setSoftZbufferEpsilon (float epsilon) { mEpsilon = epsilon; }

  bool getEwaFilter() const { return mEwaFilter; }
  void setEwaFilter (bool enable);

  float getEwaRadius() const { return mEwaRadius; }
  void setEwaRadius (float ewaRadius) { mEwaRadius = ewaRadius; }
  unsigned int getPointSizeType() const { return mPointSizeType; }
  void setPointSizeType (unsigned int pointSizeType);

  float getRadiusScale() const { return mRadiusScale; }
  void setRadiusScale (float radiusScale) { mRadiusScale = radiusScale; }

  void setupProgramObjects();
  void setupFilterKernel();
  void setupVertexArrayBuffer();
  void setupScreenQuad();
  void setupUniforms (cProgram& program);

  void renderPass (bool depth_only);

  //{{{  vars
  bool mSmooth;
  bool mSoftZbuffer;
  float mEpsilon;
  bool mEwaFilter;
  float mEwaRadius;
  unsigned int mPointSizeType;
  float mRadiusScale;

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
  //}}}
  };
