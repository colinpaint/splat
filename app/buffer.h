#pragma once
#include <GL/glew.h>

// cVertexArray
class cVertexArray {
public:
  cVertexArray();
  ~cVertexArray();
  void bind();
  void unbind();
private:
  GLuint mVertexArray;
  };

// cArray
class cArray {
public:
  cArray();
  ~cArray();
  void bind();
  void unbind();
  void set (GLsizeiptr size, GLvoid const* ptr);
private:
  GLuint mArray;
  };

// cElementArray
class cElementArray {
public:
  cElementArray();
  ~cElementArray();
  void bind();
  void unbind();
  void set (GLsizeiptr size, GLvoid const* ptr);
private:
  GLuint mElementArray;
  };

// cUniform
class cUniform {
public:
  cUniform();
  cUniform (GLsizeiptr size);
  ~cUniform();
  void bindBufferBase (GLuint index);
protected:
  void bind();
  void unbind();
private:
  GLuint mUniform;
  };
