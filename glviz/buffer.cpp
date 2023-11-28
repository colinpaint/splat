//{{{  includes
#include "buffer.h"

#include <cassert>

using namespace std;
//}}}

namespace GLviz {
  // glVertexArray
  glVertexArray::glVertexArray() { glGenVertexArrays (1, &m_vertex_array_obj); }
  glVertexArray::~glVertexArray() { glDeleteVertexArrays (1, &m_vertex_array_obj); }

  void glVertexArray::bind() { glBindVertexArray (m_vertex_array_obj); }
  void glVertexArray::unbind() { glBindVertexArray (0); }


  // glArrayBuffer
  glArrayBuffer::glArrayBuffer() { glGenBuffers (1, &m_array_buffer_obj); }
  glArrayBuffer::~glArrayBuffer() { glDeleteBuffers (1, &m_array_buffer_obj); }

  void glArrayBuffer::bind() { glBindBuffer (GL_ARRAY_BUFFER, m_array_buffer_obj); }
  void glArrayBuffer::unbind() { glBindBuffer (GL_ARRAY_BUFFER, 0); }
  //{{{
  void glArrayBuffer::set_buffer_data (GLsizeiptr size, GLvoid const* ptr) {

    bind();
    glBufferData (GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
    glBufferSubData (GL_ARRAY_BUFFER, 0, size, ptr);
    unbind();
    }
  //}}}


  // glUniformBuffer
  glUniformBuffer::glUniformBuffer() { glGenBuffers (1, &m_uniform_buffer_obj); }
  //{{{
  glUniformBuffer::glUniformBuffer(GLsizeiptr size) : glUniformBuffer() {

    glBindBuffer (GL_UNIFORM_BUFFER, m_uniform_buffer_obj);
    glBufferData (GL_UNIFORM_BUFFER, size, reinterpret_cast<GLfloat*>(0), GL_DYNAMIC_DRAW);
    glBindBuffer (GL_UNIFORM_BUFFER, 0);
    }
  //}}}
  glUniformBuffer::~glUniformBuffer() { glDeleteBuffers (1, &m_uniform_buffer_obj); }

  //{{{
  void glUniformBuffer::bind_buffer_base (GLuint index) {
    glBindBufferBase (GL_UNIFORM_BUFFER, index, m_uniform_buffer_obj);
    }
  //}}}
  void glUniformBuffer::bind() { glBindBuffer( GL_UNIFORM_BUFFER, m_uniform_buffer_obj); }
  void glUniformBuffer::unbind() { glBindBuffer (GL_UNIFORM_BUFFER, 0); }


  // glElementArrayBuffer
  glElementArrayBuffer::glElementArrayBuffer() { glGenBuffers (1, &m_element_array_buffer_obj); }
  glElementArrayBuffer::~glElementArrayBuffer() { glDeleteBuffers (1, &m_element_array_buffer_obj); }

  void glElementArrayBuffer::bind() { glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_element_array_buffer_obj); }
  void glElementArrayBuffer::unbind() { glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0); }
  //{{{
  void glElementArrayBuffer::set_buffer_data (GLsizeiptr size, GLvoid const* ptr) {

    bind();
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
    glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, 0, size, ptr);
    unbind();
    }
  //}}}
  }
