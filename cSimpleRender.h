#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <Eigen/Core>
#include "glviz/shader.h"
#include "glviz/program.h"
#include "glviz/buffer.h"
#include "glviz/camera.h"

class cSimpleRender {
public:
  cSimpleRender (GLviz::Camera const& camera);
  virtual ~cSimpleRender();

  void drawMesh3 (int shadingMethod, GLsizei nf);
  void drawSpheres (GLsizei nv);

  // vars
  GLviz::Camera const& mCamera;

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
