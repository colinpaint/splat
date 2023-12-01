#pragma once
#include "cRender.h"

class cMeshRender : public cRender {
public:
  cMeshRender (GLviz::Camera const& camera);
  virtual ~cMeshRender();

  virtual void bindUniforms() final;

  virtual void gui() final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void display (cModel* model) final;

  // vars
  GLviz::glVertexArray mVertexArrayV;
  GLviz::glVertexArray mVertexArrayVf;
  GLviz::glVertexArray mVertexArrayVnf;
  GLviz::glArrayBuffer mVertexArrayBuffer;
  GLviz::glArrayBuffer normal_array_buffer;
  GLviz::glElementArrayBuffer mIndexArrayBuffer;

  GLviz::UniformBufferCamera mUniformCamera;
  GLviz::UniformBufferMaterial mUniformMaterial;
  GLviz::UniformBufferWireFrame mUniformWireFrame;
  GLviz::UniformBufferSphere mUniformWireSphere;

  GLviz::ProgramMesh3 program_mesh3;
  GLviz::ProgramSphere program_sphere;

  int mShadingMethod = 0;
  float mProjectionRadius = 0.0f;

  bool mEnableMesh3 = true;
  float mMeshMaterial[4] = { 0.0f, 0.25f, 1.0f, 8.0f };

  bool mEnableWireFrame = false;
  float mWireFrameMaterial[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

  bool mEnableSpheres = false;
  float mPointRadius = 0.0014f;
  float mPointsMaterial[4] = { 1.0f, 1.0f, 1.0f, 8.0f };

private:
  void renderMesh (int shadingMethod, GLsizei nf);
  void renderSpheres (GLsizei nv);
  };
