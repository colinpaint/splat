//{{{  includes
#include "buffer.h"

#include <cassert>

using namespace std;
//}}}

// glVertexArray
cVertexArray::cVertexArray() { glGenVertexArrays (1, &m_vertex_array_obj); }
cVertexArray::~cVertexArray() { glDeleteVertexArrays (1, &m_vertex_array_obj); }

void cVertexArray::bind() { glBindVertexArray (m_vertex_array_obj); }
void cVertexArray::unbind() { glBindVertexArray (0); }


// glArrayBuffer
cArrayBuffer::cArrayBuffer() { glGenBuffers (1, &m_array_buffer_obj); }
cArrayBuffer::~cArrayBuffer() { glDeleteBuffers (1, &m_array_buffer_obj); }

void cArrayBuffer::bind() { glBindBuffer (GL_ARRAY_BUFFER, m_array_buffer_obj); }
void cArrayBuffer::unbind() { glBindBuffer (GL_ARRAY_BUFFER, 0); }
//{{{
void cArrayBuffer::set (GLsizeiptr size, GLvoid const* ptr) {
  bind();
  glBufferData (GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
  glBufferSubData (GL_ARRAY_BUFFER, 0, size, ptr);
  unbind();
  }
//}}}


// glElementArrayBuffer
cElementArrayBuffer::cElementArrayBuffer() { glGenBuffers (1, &m_element_array_buffer_obj); }
cElementArrayBuffer::~cElementArrayBuffer() { glDeleteBuffers (1, &m_element_array_buffer_obj); }

void cElementArrayBuffer::bind() { glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_element_array_buffer_obj); }
void cElementArrayBuffer::unbind() { glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0); }
//{{{
void cElementArrayBuffer::set (GLsizeiptr size, GLvoid const* ptr) {
  bind();
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
  glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, 0, size, ptr);
  unbind();
  }
//}}}


// glUniformBuffer
cUniform::cUniform() { glGenBuffers (1, &m_uniform_buffer_obj); }
//{{{
cUniform::cUniform(GLsizeiptr size) : cUniform() {

  glBindBuffer (GL_UNIFORM_BUFFER, m_uniform_buffer_obj);
  glBufferData (GL_UNIFORM_BUFFER, size, reinterpret_cast<GLfloat*>(0), GL_DYNAMIC_DRAW);
  glBindBuffer (GL_UNIFORM_BUFFER, 0);
  }
//}}}
cUniform::~cUniform() { glDeleteBuffers (1, &m_uniform_buffer_obj); }

//{{{
void cUniform::bindBufferBase (GLuint index) {
  glBindBufferBase (GL_UNIFORM_BUFFER, index, m_uniform_buffer_obj);
  }
//}}}
void cUniform::bind() { glBindBuffer( GL_UNIFORM_BUFFER, m_uniform_buffer_obj); }
void cUniform::unbind() { glBindBuffer (GL_UNIFORM_BUFFER, 0); }
