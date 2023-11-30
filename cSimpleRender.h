#pragma once
#include "cRender.h"

class cSimpleRender : public cRender {
public:
  cSimpleRender (GLviz::Camera const& camera);
  virtual ~cSimpleRender();

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
  GLviz::UniformBufferWireframe uniform_wireframe;
  GLviz::UniformBufferSphere uniform_sphere;

  GLviz::ProgramMesh3 program_mesh3;
  GLviz::ProgramSphere program_sphere;

  bool mEnableMesh3 = true;
  bool mEnableWireframe = false;
  bool mEnableSpheres = false;

  int mShadingMethod = 0;
  float mPointRadius = 0.0014f;
  float mProjectionRadius = 0.0f;
  float mWireframe[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  float mMeshMaterial[4] = { 0.0f, 0.25f, 1.0f, 8.0f };
  float mPointsMaterial[4] = { 1.0f, 1.0f, 1.0f, 8.0f };

private:
  void drawMesh3 (int shadingMethod, GLsizei nf);
  void drawSpheres (GLsizei nv);
  };
