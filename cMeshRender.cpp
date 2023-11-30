//{{{  includes
#include "cMeshRender.h"

#include <Eigen/Core>

#include "../common/cLog.h"

#include "glviz/glviz.h"
#include "glviz/utility.h"

using namespace std;
//}}}

//{{{
cMeshRender::cMeshRender (GLviz::Camera const& camera) : cRender(camera) {

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
  uniform_wireFrame.bindBufferBase (2);
  uniform_sphere.bindBufferBase (3);
  }
//}}}
cMeshRender::~cMeshRender() {}

void cMeshRender::setMultiSample (bool enable) {}
void cMeshRender::setBackFaceCull (bool enable) {}

void cMeshRender::resize (int width, int height) {}
//{{{
void cMeshRender::render (cModel* model) {

  if (getMultiSample()) 
    glEnable (GL_MULTISAMPLE);
  glEnable (GL_DEPTH_TEST);

  glClearDepth (1.0f);
  glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  vertex_array_buffer.set_buffer_data (3 * sizeof(GLfloat) * model->mVertices.size(),
                                       model->mVertices.front().data());
  normal_array_buffer.set_buffer_data (3 * sizeof(GLfloat) * model->mNormals.size(),
                                       model->mNormals.front().data());
  index_array_buffer.set_buffer_data (3 * sizeof(GLuint) * model->mFaces.size(),
                                      model->mFaces.front().data());

  uniform_camera.set_buffer_data (mCamera);

  if (mEnableMesh3) {
    uniform_material.set_buffer_data (mMeshMaterial);
    program_mesh3.set_wireFrame (mEnableWireFrame);
    int screen[2] = { GLviz::getScreenWidth(), GLviz::getScreenHeight() };
    uniform_wireFrame.set_buffer_data (mWireFrameMaterial, screen);
    program_mesh3.set_smooth (mShadingMethod != 0);
    renderMesh (mShadingMethod, static_cast<GLsizei>(3 * model->mFaces.size()));
    }

  if (mEnableSpheres) {
    uniform_material.set_buffer_data (mPointsMaterial);
    GLviz::Frustum view_frustum = mCamera.get_frustum();
    mProjectionRadius = view_frustum.near_() *
                          (GLviz::getScreenHeight() / (view_frustum.top() - view_frustum.bottom()));
    uniform_sphere.set_buffer_data (mPointRadius, mProjectionRadius);
    renderSpheres (static_cast<GLsizei>(model->mVertices.size()));
    }

  glDisable (GL_DEPTH_TEST);
  if (getMultiSample()) 
    glDisable (GL_MULTISAMPLE);
  }
//}}}
//{{{
bool cMeshRender::keyboard (SDL_Keycode key) {

  switch (key) {
    case SDLK_1: mEnableMesh3 = !mEnableMesh3; return true;
    case SDLK_2: mEnableSpheres = !mEnableSpheres; return true;
    case SDLK_5: mShadingMethod = (mShadingMethod + 1) % 2; return true;
    case SDLK_w: mEnableWireFrame = !mEnableWireFrame; return true;
    }

  return false;
  }
//}}}
//{{{
void cMeshRender::gui() {

  if (ImGui::CollapsingHeader ("Mesh", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox ("Mesh draw", &(mEnableMesh3));
    ImGui::ColorEdit3 ("Mesh color", mMeshMaterial);
    ImGui::DragFloat ("Mesh shinines", &(mMeshMaterial[3]), 1e-2f, 1e-12f, 1000.0f);
    ImGui::Combo ("Mesh shading", &mShadingMethod, "Flat\0Phong\0\0");

    ImGui::Separator();
    ImGui::Checkbox ("WireFrame draw", &mEnableWireFrame);
    ImGui::ColorEdit3 ("WireFrame color", mWireFrameMaterial);
    }

  if (ImGui::CollapsingHeader ("Spheres", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox ("Spheres draw", &mEnableSpheres);
    ImGui::DragFloat ("Spheres radius", &(mPointRadius), 1e-5f, 0.0f, 0.1f, "%.4f");
    ImGui::ColorEdit3 ("Spheres color", mPointsMaterial);
    ImGui::DragFloat ("Spheres shininess", &mPointsMaterial[3], 1e-2f, 1e-12f, 1000.0f);
    }
  }
//}}}

// private
//{{{
void cMeshRender::renderMesh (int shadingMethod, GLsizei nf) {

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
void cMeshRender::renderSpheres (GLsizei nv) {

  glEnable (GL_PROGRAM_POINT_SIZE);
  glPointParameterf (GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

  program_sphere.use();

  vertex_array_v.bind();
  glDrawArrays (GL_POINTS, 0, nv);
  vertex_array_v.unbind();

  program_sphere.unuse();
  glDisable (GL_PROGRAM_POINT_SIZE);
  }
//}}}
