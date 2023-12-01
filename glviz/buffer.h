#pragma once
#include <GL/glew.h>

namespace GLviz {
  //{{{
  class glVertexArray {
  public:
    glVertexArray();
    ~glVertexArray();

    void bind();
    void unbind();

  private:
    GLuint m_vertex_array_obj;
    };
  //}}}
  //{{{
  class glArrayBuffer {
  public:
    glArrayBuffer();
    ~glArrayBuffer();

    void bind();
    void unbind();

    void set_buffer_data(GLsizeiptr size, GLvoid const* ptr);

  private:
    GLuint m_array_buffer_obj;
    };
  //}}}
  //{{{
  class glElementArrayBuffer {
  public:
    glElementArrayBuffer();
    ~glElementArrayBuffer();

    void bind();
    void unbind();

    void set_buffer_data(GLsizeiptr size, GLvoid const* ptr);

  private:
    GLuint m_element_array_buffer_obj;
    };
  //}}}
  //{{{
  class glUniformBuffer {
  public:
    glUniformBuffer();
    glUniformBuffer(GLsizeiptr size);

    ~glUniformBuffer();

    void bindBufferBase (GLuint index);

  protected:
    void bind();
    void unbind();

  private:
    GLuint m_uniform_buffer_obj;
    };
  //}}}
  }
