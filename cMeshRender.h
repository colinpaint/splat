#pragma once
#include "cRender.h"
#include "glviz/buffer.h"
#include "glviz/shader.h"

//{{{
class cUniformMaterial : public GLviz::glUniformBuffer {
public:
  cUniformMaterial();
  void setBuffer (const float* mbuf);
  };
//}}}
//{{{
class cUniformWireFrame : public GLviz::glUniformBuffer {
public:
  cUniformWireFrame();
  void setBuffer(float const* color, int const* viewport);
  };
//}}}
//{{{
class cUniformSphere : public GLviz::glUniformBuffer {
public:
  cUniformSphere();
  void setBuffer (float radius, float projection);
  };
//}}}

//{{{
class cProgramMesh : public glProgram {
public:
  cProgramMesh();

  void setWireFrame (bool enable);
  void setSmooth (bool enable);

private:
  void initShader();
  void initProgram();

  glVertexShader mMeshVs;
  glGeometryShader mMeshGs;
  glFragmentShader mMeshFs;

  bool mWireFrame;
  bool mSmooth;
  };
//}}}
//{{{
class cProgramSphere : public glProgram {
public:
  cProgramSphere();

private:
  void initShader();
  void initProgram();

  glVertexShader mSphereVs;
  glFragmentShader mSphereFs;
  };
//}}}


class cMeshRender : public cRender {
public:
  cMeshRender (GLviz::cCamera const& camera);
  virtual ~cMeshRender();

  virtual void use (bool multiSample, bool backFaceCull) final;

  virtual void gui() final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void display (cModel* model) final;

  // vars
  GLviz::glVertexArray mVertexArrayV;
  GLviz::glVertexArray mVertexArrayVf;
  GLviz::glVertexArray mVertexArrayVnf;
  GLviz::glArrayBuffer mVertexArrayBuffer;
  GLviz::glArrayBuffer mNormalArrayBuffer;
  GLviz::glElementArrayBuffer mIndexArrayBuffer;

  cUniformMaterial mUniformMaterial;
  cUniformWireFrame mUniformWireFrame;
  cUniformSphere mUniformWireSphere;

  cProgramMesh mProgramMesh;
  cProgramSphere mProgramSphere;

  int mShadingMethod = 0;
  float mProjectionRadius = 0.0f;

  bool mDisplayMesh = true;
  float mMeshMaterial[4] = { 0.0f, 0.25f, 1.0f, 8.0f };

  bool mDisplayWireFrame = false;
  float mWireFrameMaterial[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

  bool mDisplaySpheres = false;
  float mPointRadius = 0.0014f;
  float mPointsMaterial[4] = { 1.0f, 1.0f, 1.0f, 8.0f };
  };
