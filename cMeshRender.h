#pragma once
#include "cRender.h"

class cMeshRender : public cRender {
public:
  cMeshRender (GLviz::Camera const& camera);
  virtual ~cMeshRender();

  virtual void setBackFaceCull (bool enable = true) final;
  virtual void setMultiSample (bool enable = true) final;

  virtual void resize (int width, int height) final;
  virtual void render (cModel* model) final;
  virtual bool keyboard (SDL_Keycode key) final;
  virtual void gui() final;

  // vars
  GLviz::glVertexArray vertex_array_v;
  GLviz::glVertexArray vertex_array_vf;
  GLviz::glVertexArray vertex_array_vnf;

  GLviz::glArrayBuffer vertex_array_buffer;
  GLviz::glArrayBuffer normal_array_buffer;

  GLviz::glElementArrayBuffer index_array_buffer;

  GLviz::UniformBufferCamera uniform_camera;
  GLviz::UniformBufferMaterial uniform_material;
  GLviz::UniformBufferWireFrame uniform_wireFrame;
  GLviz::UniformBufferSphere uniform_sphere;

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
