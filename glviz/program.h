#pragma once
#include "shader.h"
#include "buffer.h"

namespace GLviz {
  class Camera;
  //{{{
  class UniformBufferCamera : public glUniformBuffer {
  public:
    UniformBufferCamera();

    void set_buffer_data (Camera const& camera);
    };
  //}}}
  //{{{
  class UniformBufferMaterial : public glUniformBuffer {
  public:
    void set_buffer_data(const float* mbuf);
    };
  //}}}
  //{{{
  class UniformBufferWireframe : public glUniformBuffer {
  public:
    UniformBufferWireframe();

    void set_buffer_data(float const* color, int const* viewport);
    };
  //}}}
  //{{{
  class UniformBufferSphere : public glUniformBuffer {
  public:
    UniformBufferSphere();

    void set_buffer_data (float radius, float projection);
    };
  //}}}

  //{{{
  class ProgramMesh3 : public glProgram {
  public:
    ProgramMesh3();

    void set_wireframe (bool enable);
    void set_smooth (bool enable);

  private:
    void initShader();
    void initProgram();

    glVertexShader m_mesh3_vs_obj;
    glGeometryShader m_mesh3_gs_obj;
    glFragmentShader m_mesh3_fs_obj;

    bool m_wireframe;
    bool m_smooth;
    };
  //}}}
  //{{{
  class ProgramSphere : public glProgram {
  public:
    ProgramSphere();

  private:
    void initShader();
    void initProgram();

    glVertexShader m_sphere_vs_obj;
    glFragmentShader m_sphere_fs_obj;
    };
  //}}}
  }
