#pragma once
#include "cRender.h"

class cSimpleRender : public cRender {
public:
  cSimpleRender (GLviz::Camera const& camera);
  virtual ~cSimpleRender();

  void drawMesh3 (int shadingMethod, GLsizei nf);
  void drawSpheres (GLsizei nv);

  virtual void setBackFaceCull (bool enable = true) final;
  virtual void setMultiSample (bool enable = true) final;

  virtual void resize (int width, int height) final;
  virtual void render (cSurfelModel& model) final;
  virtual bool keyboard (SDL_Keycode key) final;

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
  };
