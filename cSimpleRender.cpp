//{{{  includes
#include <memory>
#include <Eigen/Core>

#include "cSimpleRender.h"

#include "glviz/glviz.h"
#include "glviz/utility.h"

#include "../common/cLog.h"

using namespace std;
//}}}

//{{{
cSimpleRender::cSimpleRender (GLviz::Camera const& camera) : mCamera(camera) {

  // Setup vertex array v
  vertex_array_v.bind();

  vertex_array_buffer.bind();
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

  vertex_array_v.unbind();

  // Setup vertex array vf.
  vertex_array_vf.bind();

  vertex_array_buffer.bind();
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

  index_array_buffer.bind();
  vertex_array_buffer.unbind();
  vertex_array_vf.unbind();

  // Setup vertex array vnf.
  vertex_array_vnf.bind();

  vertex_array_buffer.bind();
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

  normal_array_buffer.bind();
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

  index_array_buffer.bind();
  vertex_array_buffer.unbind();

  vertex_array_vnf.unbind();

  // Bind uniforms to their binding points
  uniform_camera.bindBufferBase (0);
  uniform_material.bindBufferBase (1);
  uniform_wireframe.bindBufferBase (2);
  uniform_sphere.bindBufferBase (3);
  }
//}}}
cSimpleRender::~cSimpleRender() {}

//{{{
void cSimpleRender::drawMesh3 (int shadingMethod, GLsizei nf) {

  program_mesh3.use();

  if (shadingMethod == 0) {
    // Flat
    vertex_array_vf.bind();
    glDrawElements (GL_TRIANGLES, nf, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
    vertex_array_vf.unbind();
    }
  else {
    // Smooth
    vertex_array_vnf.bind();
    glDrawElements (GL_TRIANGLES, nf, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
    vertex_array_vnf.unbind();
    }

  program_mesh3.unuse();
  }
//}}}
//{{{
void cSimpleRender::drawSpheres (GLsizei nv) {

  glEnable (GL_PROGRAM_POINT_SIZE);
  glPointParameterf (GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

  program_sphere.use();

  vertex_array_v.bind();
  glDrawArrays (GL_POINTS, 0, nv);
  vertex_array_v.unbind();

  program_sphere.unuse();
  }
//}}}
