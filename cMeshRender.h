#pragma once
#include "cRender.h"
#include "app/buffer.h"
#include "app/shader.h"

//{{{
class cUniformMaterial : public cUniform {
public:
  cUniformMaterial();
  void set (const float* mbuf);
  };
//}}}
//{{{
class cUniformWireFrame : public cUniform {
public:
  cUniformWireFrame();
  void set(float const* color, int const* viewport);
  };
//}}}
//{{{
class cUniformSphere : public cUniform {
public:
  cUniformSphere();
  void set (float radius, float projection);
  };
//}}}

//{{{
class cProgramMesh : public cProgram {
public:
  cProgramMesh();

  void setWireFrame (bool enable);
  void setSmooth (bool enable);

private:
  void initShader();
  void initProgram();

  cVertexShader mMeshVs;
  cGeometryShader mMeshGs;
  cFragmentShader mMeshFs;

  bool mWireFrame;
  bool mSmooth;
  };
//}}}
//{{{
class cProgramSphere : public cProgram {
public:
  cProgramSphere();

private:
  void initShader();
  void initProgram();

  cVertexShader mSphereVs;
  cFragmentShader mSphereFs;
  };
//}}}

class cMeshRender : public cRender {
public:
  cMeshRender (cApp& app);
  virtual ~cMeshRender();

  virtual void use (bool multiSample, bool backFaceCull) final;

  virtual void gui() final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void display (cModel* model) final;

  // vars
  int mShadingMethod = 0;
  float mProjectionRadius = 0.0f;

  bool mDisplayMesh = true;
  float mMeshMaterial[4] = { 0.0f, 0.25f, 1.0f, 8.0f };

  bool mDisplayWireFrame = false;
  float mWireFrameMaterial[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

  bool mDisplaySpheres = false;
  float mPointRadius = 0.0014f;
  float mPointsMaterial[4] = { 1.0f, 1.0f, 1.0f, 8.0f };

  cVertexArray mVertexArrayV;
  cVertexArray mVertexArrayVf;
  cVertexArray mVertexArrayVnf;
  cArray mVertexArrayBuffer;
  cArray mNormalArrayBuffer;
  cElementArray mIndexArrayBuffer;

  cUniformMaterial mUniformMaterial;
  cUniformWireFrame mUniformWireFrame;
  cUniformSphere mUniformWireSphere;

  cProgramMesh mProgramMesh;
  cProgramSphere mProgramSphere;
  };
