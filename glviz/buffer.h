#pragma once
#include <GL/glew.h>

namespace GLviz {
  //{{{
  class cVertexArray {
  public:
    cVertexArray();
    ~cVertexArray();

    void bind();
    void unbind();

  private:
    GLuint m_vertex_array_obj;
    };
  //}}}
  //{{{
  class cArrayBuffer {
  public:
    cArrayBuffer();
    ~cArrayBuffer();

    void bind();
    void unbind();
    void set(GLsizeiptr size, GLvoid const* ptr);

  private:
    GLuint m_array_buffer_obj;
    };
  //}}}
  //{{{
  class cElementArrayBuffer {
  public:
    cElementArrayBuffer();
    ~cElementArrayBuffer();

    void bind();
    void unbind();
    void set(GLsizeiptr size, GLvoid const* ptr);

  private:
    GLuint m_element_array_buffer_obj;
    };
  //}}}
  //{{{
  class cUniformBuffer {
  public:
    cUniformBuffer();
    cUniformBuffer(GLsizeiptr size);

    ~cUniformBuffer();

    void bindBufferBase (GLuint index);

  protected:
    void bind();
    void unbind();

  private:
    GLuint m_uniform_buffer_obj;
    };
  //}}}
  }
