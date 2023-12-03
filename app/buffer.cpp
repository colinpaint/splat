//{{{  includes
#include "buffer.h"

#include <cassert>

using namespace std;
//}}}

// cVertexArray
cVertexArray::cVertexArray() { glGenVertexArrays (1, &mVertexArray); }
cVertexArray::~cVertexArray() { glDeleteVertexArrays (1, &mVertexArray); }

void cVertexArray::bind() { glBindVertexArray (mVertexArray); }
void cVertexArray::unbind() { glBindVertexArray (0); }

// cArray
cArray::cArray() { glGenBuffers (1, &mArray); }
cArray::~cArray() { glDeleteBuffers (1, &mArray); }

void cArray::bind() { glBindBuffer (GL_ARRAY_BUFFER, mArray); }
void cArray::unbind() { glBindBuffer (GL_ARRAY_BUFFER, 0); }
//{{{
void cArray::set (GLsizeiptr size, GLvoid const* ptr) {
  bind();
  glBufferData (GL_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
  glBufferSubData (GL_ARRAY_BUFFER, 0, size, ptr);
  unbind();
  }
//}}}

// cElementArray
cElementArray::cElementArray() { glGenBuffers (1, &mElementArray); }
cElementArray::~cElementArray() { glDeleteBuffers (1, &mElementArray); }

void cElementArray::bind() { glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mElementArray); }
void cElementArray::unbind() { glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0); }
//{{{
void cElementArray::set (GLsizeiptr size, GLvoid const* ptr) {
  bind();
  glBufferData (GL_ELEMENT_ARRAY_BUFFER, size, nullptr, GL_STREAM_DRAW);
  glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, 0, size, ptr);
  unbind();
  }
//}}}

// cUniform
cUniform::cUniform() { glGenBuffers (1, &mUniform); }
//{{{
cUniform::cUniform(GLsizeiptr size) : cUniform() {

  glBindBuffer (GL_UNIFORM_BUFFER, mUniform);
  glBufferData (GL_UNIFORM_BUFFER, size, reinterpret_cast<GLfloat*>(0), GL_DYNAMIC_DRAW);
  glBindBuffer (GL_UNIFORM_BUFFER, 0);
  }
//}}}
cUniform::~cUniform() { glDeleteBuffers (1, &mUniform); }

//{{{
void cUniform::bindBufferBase (GLuint index) {
  glBindBufferBase (GL_UNIFORM_BUFFER, index, mUniform);
  }
//}}}
void cUniform::bind() { glBindBuffer( GL_UNIFORM_BUFFER, mUniform); }
void cUniform::unbind() { glBindBuffer (GL_UNIFORM_BUFFER, 0); }
