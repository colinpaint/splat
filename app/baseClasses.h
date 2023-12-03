#pragma once
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <GL/glew.h>

//{{{
class cVertexArray {
public:
  cVertexArray();
  ~cVertexArray();
  void bind();
  void unbind();
private:
  GLuint mVertexArray;
  };
//}}}
//{{{
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
//}}}
//{{{
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
//}}}
//{{{
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
//}}}

//{{{
class cShader {
public:
  virtual ~cShader();

  void load (const std::vector <std::string>& source);
  void loadFile (std::string const& filename);

  void compile (std::map<std::string, int> const& define_list = std::map<std::string, int>());
  bool isCompiled() const;

  std::string infoLog();

protected:
  cShader();

  GLuint mShader;
  std::string m_source;

  friend class cProgram;
  };
//}}}
//{{{
class cVertexShader : public cShader {
public:
  cVertexShader();
  };
//}}}
//{{{
class cGeometryShader : public cShader {
public:
  cGeometryShader();
  };
//}}}
//{{{
class cFragmentShader : public cShader {
public:
  cFragmentShader();
  };
//}}}

//{{{
class cProgram {
public:
    cProgram();
    virtual ~cProgram();

    void use() const;
    void unuse() const;
    void link();

    void attachShader(cShader& shader);
    void detachShader(cShader& shader);

    void detachAll();

    bool isLinked();
    bool isAttached(cShader const& shader);
    std::string infoLog();

    void setUniform1i (GLchar const* name, GLint value);
    void setUniformBind (GLchar const* name, GLuint blockBind);

protected:
    GLuint mProgram;
  };
//}}}

//{{{
struct fileOpenError : public std::runtime_error {
  fileOpenError (const std::string& errmsg) : runtime_error(errmsg) { }
  };
//}}}
//{{{
struct shaderCompilationError : public std::logic_error {
  shaderCompilationError (const std::string& errmsg) : logic_error(errmsg) {}
  };
//}}}
//{{{
struct shaderLinkError : public std::logic_error {
  shaderLinkError (std::string const& errmsg) : logic_error(errmsg) {}
  };
//}}}
//{{{
struct uniformNotFoundError : public std::logic_error {
  uniformNotFoundError (std::string const& errmsg) : logic_error(errmsg) {}
  };
//}}}
