//{{{  includes
#include "program.h"
#include "camera.h"

#include <Eigen/Core>
#include <iostream>
#include <cstdlib>

using namespace Eigen;

extern unsigned char const mesh3_vs_glsl[];
extern unsigned char const mesh3_gs_glsl[];
extern unsigned char const mesh3_fs_glsl[];

extern unsigned char const sphere_vs_glsl[];
extern unsigned char const sphere_fs_glsl[];

using namespace std;
//}}}

namespace GLviz {
  //{{{
  ProgramSphere::ProgramSphere()
  {
      initialize_shader_obj();
      initialize_program_obj();
  }
  //}}}
  //{{{
  void ProgramSphere::initialize_shader_obj() {

    m_sphere_vs_obj.load_from_cstr (reinterpret_cast<char const*>(sphere_vs_glsl));
    m_sphere_fs_obj.load_from_cstr (reinterpret_cast<char const*>(sphere_fs_glsl));

    attach_shader (m_sphere_vs_obj);
    attach_shader (m_sphere_fs_obj);
    }
  //}}}
  //{{{
  void ProgramSphere::initialize_program_obj() {

    try {
      m_sphere_vs_obj.compile();
      m_sphere_fs_obj.compile();
      }
    catch (shader_compilation_error const& e) {
      cerr << "Error: A shader failed to compile." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      link();
      }
    catch (shader_link_error const& e) {
      cerr << "Error: A program failed to link." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      set_uniform_block_binding ("Camera", 0);
      set_uniform_block_binding ("Material", 1);
      set_uniform_block_binding ("Sphere", 3);
      }
    catch (uniform_not_found_error const& e) {
      cerr << "Warning: Failed to set a uniform variable." << endl << e.what() << endl;
      }
    }
  //}}}

  //{{{
  void UniformBufferWireframe::set_buffer_data (float const* color, int const* viewport) {

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), color);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLint), viewport);
    unbind();
    }
  //}}}

  UniformBufferCamera::UniformBufferCamera() : glUniformBuffer(48 * sizeof(GLfloat)) { }
  //{{{
  void UniformBufferCamera::set_buffer_data (Camera const& camera) {

    Matrix4f const& modelview_matrix = camera.get_modelview_matrix();
    Matrix4f modelview_matrix_it = modelview_matrix.inverse().transpose();
    Matrix4f const& projection_matrix = camera.get_projection_matrix();

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(GLfloat), modelview_matrix.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), 16 * sizeof(GLfloat), modelview_matrix_it.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), 16 * sizeof(GLfloat), projection_matrix.data());
    unbind();
    }
  //}}}

  //{{{
  void UniformBufferMaterial::set_buffer_data (float const* mbuf) {

    bind();
    glBufferData (GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), mbuf, GL_DYNAMIC_DRAW);
    unbind();
    }
  //}}}
  UniformBufferWireframe::UniformBufferWireframe() : glUniformBuffer(4 * sizeof(GLfloat) + 2 * sizeof(GLint)) { }
  UniformBufferSphere::UniformBufferSphere() : glUniformBuffer(2 * sizeof(GLfloat)) { }
  //{{{
  void UniformBufferSphere::set_buffer_data (float radius, float projection) {

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat), &radius);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLfloat), sizeof(GLfloat), &projection);
    unbind();
    }
  //}}}

  //{{{
  ProgramMesh3::ProgramMesh3() : glProgram(), m_wireframe(false), m_smooth(false) {

    initialize_shader_obj();
    initialize_program_obj();
    }
  //}}}
  //{{{
  void ProgramMesh3::set_wireframe (bool enable) {

    if (m_wireframe != enable) {
      m_wireframe = enable;
      initialize_program_obj();
      }
    }
  //}}}
  //{{{
  void ProgramMesh3::set_smooth (bool enable) {

    if (m_smooth != enable) {
      m_smooth = enable;
      initialize_program_obj();
      }
    }
  //}}}
  //{{{
  void ProgramMesh3::initialize_shader_obj() {

    m_mesh3_vs_obj.load_from_cstr (reinterpret_cast<char const*>(mesh3_vs_glsl));
    m_mesh3_gs_obj.load_from_cstr (reinterpret_cast<char const*>(mesh3_gs_glsl));
    m_mesh3_fs_obj.load_from_cstr (reinterpret_cast<char const*>(mesh3_fs_glsl));

    attach_shader (m_mesh3_vs_obj);
    attach_shader (m_mesh3_gs_obj);
    attach_shader (m_mesh3_fs_obj);
    }
  //}}}
  //{{{
  void ProgramMesh3::initialize_program_obj() {

    try {
      map<string, int> defines;

      defines.insert(make_pair("WIREFRAME", m_wireframe ? 1 : 0));
      defines.insert(make_pair("SMOOTH", m_smooth ? 1 : 0));

      m_mesh3_vs_obj.compile(defines);
      m_mesh3_gs_obj.compile(defines);
      m_mesh3_fs_obj.compile(defines);
      }
    catch (shader_compilation_error const& e) {
      cerr << "Error: A shader failed to compile." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      link();
      }
    catch (shader_link_error const& e) {
      cerr << "Error: A program failed to link." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      set_uniform_block_binding("Camera", 0);
      set_uniform_block_binding("Material", 1);

      if (m_wireframe)
        set_uniform_block_binding("Wireframe", 2);
      }
    catch (uniform_not_found_error const& e) {
      cerr << "Warning: Failed to set a uniform variable." << endl << e.what() << endl;
      }
    }
  //}}}
  }
