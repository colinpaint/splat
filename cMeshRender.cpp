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

  // setup vertex array v
  mVertexArrayV.bind();

  mVertexArrayBuffer.bind();
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

  mVertexArrayV.unbind();

  // setup vertex array vf
  mVertexArrayVf.bind();
  mVertexArrayBuffer.bind();

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
  mIndexArrayBuffer.bind();

  mVertexArrayBuffer.unbind();
  mVertexArrayVf.unbind();

  // setup vertex array vnf
  mVertexArrayVnf.bind();
  mVertexArrayBuffer.bind();

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
  mNormalArrayBuffer.bind();

  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
  mIndexArrayBuffer.bind();

  mVertexArrayBuffer.unbind();
  mVertexArrayVnf.unbind();

  bindUniforms();
  }
//}}}
cMeshRender::~cMeshRender() {}

//{{{
void cMeshRender::bindUniforms() {
// bind uniforms to binding points

  mUniformCamera.bindBufferBase (0);
  mUniformMaterial.bindBufferBase (1);
  mUniformWireFrame.bindBufferBase (2);
  mUniformWireSphere.bindBufferBase (3);
  }
//}}}

//{{{
void cMeshRender::gui() {

  if (ImGui::CollapsingHeader ("Mesh", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox ("draw", &mDisplayMesh);
    ImGui::ColorEdit3 ("color", mMeshMaterial);
    ImGui::DragFloat ("shine", &(mMeshMaterial[3]), 1e-2f, 1e-12f, 1000.0f);
    ImGui::Combo ("shading", &mShadingMethod, "flat\0phong\0\0");
    ImGui::Separator();

    ImGui::Checkbox ("wireFrame", &mDisplayWireFrame);
    ImGui::ColorEdit3 ("color##wire", mWireFrameMaterial);
    ImGui::Separator();
    }

 if (ImGui::CollapsingHeader ("Spheres", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
   ImGui::Checkbox ("draw##spheres", &mDisplaySpheres);
   ImGui::DragFloat ("radius##spheres", &mPointRadius, 1e-5f, 0.0f, 0.1f, "%.4f");
   ImGui::ColorEdit3 ("color##spheres", mPointsMaterial);
   ImGui::DragFloat ("shine##spheres", &mPointsMaterial[3], 1e-2f, 1e-12f, 1000.0f);
   }
 }
//}}}
//{{{
bool cMeshRender::keyboard (SDL_Keycode key) {

  switch (key) {
    case SDLK_1: mDisplayMesh = !mDisplayMesh; return true;
    case SDLK_2: mDisplaySpheres = !mDisplaySpheres; return true;
    case SDLK_5: mShadingMethod = (mShadingMethod + 1) % 2; return true;
    case SDLK_w: mDisplayWireFrame = !mDisplayWireFrame; return true;
    }

  return false;
  }
//}}}
//{{{
void cMeshRender::display (cModel* model) {

  if (getMultiSample())
    glEnable (GL_MULTISAMPLE);

  glEnable (GL_DEPTH_TEST);

  glDepthMask (GL_TRUE);
  glClearDepth (1.0f);
  glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mVertexArrayBuffer.set_buffer_data (3 * model->getNumVertices() * sizeof(GLfloat), model->getVerticesData());
  mNormalArrayBuffer.set_buffer_data (3 * model->getNumNormals() * sizeof(GLfloat), model->getNormalsData());
  mIndexArrayBuffer.set_buffer_data (3 * model->getNumFaces() * sizeof(GLfloat), model->getFacesData());

  mUniformCamera.set_buffer_data (mCamera);

  if (mDisplayMesh) {
    mUniformMaterial.set_buffer_data (mMeshMaterial);

    array <int,2> screen = { GLviz::getScreenWidth(), GLviz::getScreenHeight() };
    mUniformWireFrame.set_buffer_data (mWireFrameMaterial, screen.data());

    //{{{  display mesh
    mProgramMesh.set_wireFrame (mDisplayWireFrame);
    mProgramMesh.set_smooth (mShadingMethod != 0);

    mProgramMesh.use();

    if (mShadingMethod == 0) {
      // Flat
      mVertexArrayVf.bind();
      glDrawElements (GL_TRIANGLES, (GLsizei)model->getNumFaces() * 3, 
                      GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
      mVertexArrayVf.unbind();
      }
    else {
      // Smooth
      mVertexArrayVnf.bind();
      glDrawElements (GL_TRIANGLES, (GLsizei)model->getNumFaces() * 3, 
                      GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
      mVertexArrayVnf.unbind();
      }

    mProgramMesh.unuse();
    //}}}
    }

  if (mDisplaySpheres) {
    mUniformMaterial.set_buffer_data (mPointsMaterial);

    GLviz::Frustum view_frustum = mCamera.get_frustum();
    mProjectionRadius =
      view_frustum.near_() * (GLviz::getScreenHeight() / (view_frustum.top() - view_frustum.bottom()));
    mUniformWireSphere.set_buffer_data (mPointRadius, mProjectionRadius);

    //{{{  display spheres
    glEnable (GL_PROGRAM_POINT_SIZE);
    glPointParameterf (GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    mProgramSphere.use();
    mVertexArrayV.bind();
    glDrawArrays (GL_POINTS, 0, static_cast<GLsizei>(model->getNumVertices()));
    mVertexArrayV.unbind();
    mProgramSphere.unuse();

    glDisable (GL_PROGRAM_POINT_SIZE);
    //}}}
    }

  glDisable (GL_DEPTH_TEST);

  if (getMultiSample())
    glDisable (GL_MULTISAMPLE);
  }
//}}}
