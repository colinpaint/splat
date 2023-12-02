#pragma once
#include "cRender.h"
#include "glviz/buffer.h"
#include "glviz/shader.h"

//{{{
class cUniformBufferMaterial : public GLviz::glUniformBuffer {
public:
  void set_buffer_data(const float* mbuf);
  };
//}}}
//{{{
class cUniformBufferWireFrame : public GLviz::glUniformBuffer {
public:
  cUniformBufferWireFrame();

  void set_buffer_data(float const* color, int const* viewport);
  };
//}}}
//{{{
class cUniformBufferSphere : public GLviz::glUniformBuffer {
public:
  cUniformBufferSphere();

  void set_buffer_data (float radius, float projection);
  };
//}}}

//{{{
class cProgramMesh : public glProgram {
public:
  cProgramMesh();

  void set_wireFrame (bool enable);
  void set_smooth (bool enable);

private:
  void initShader();
  void initProgram();

  glVertexShader m_mesh_vs_obj;
  glGeometryShader m_mesh_gs_obj;
  glFragmentShader m_mesh_fs_obj;

  bool m_wireFrame;
  bool m_smooth;
  };
//}}}
//{{{
class cProgramSphere : public glProgram {
public:
  cProgramSphere();

private:
  void initShader();
  void initProgram();

  glVertexShader m_sphere_vs_obj;
  glFragmentShader m_sphere_fs_obj;
  };
//}}}

class cMeshRender : public cRender {
public:
  cMeshRender (GLviz::Camera const& camera);
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

  GLviz::UniformBufferCamera mUniformCamera;

  cUniformBufferMaterial mUniformMaterial;
  cUniformBufferWireFrame mUniformWireFrame;
  cUniformBufferSphere mUniformWireSphere;

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
