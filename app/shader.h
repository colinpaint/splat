#pragma once
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <GL/glew.h>

//{{{
struct file_open_error : public std::runtime_error {
  file_open_error (const std::string& errmsg) : runtime_error(errmsg) { }
  };
//}}}
//{{{
struct shader_compilation_error : public std::logic_error {
  shader_compilation_error (const std::string& errmsg) : logic_error(errmsg) { }
  };
//}}}
//{{{
struct shader_link_error : public std::logic_error {
  shader_link_error (std::string const& errmsg) : logic_error(errmsg) { }
  };
//}}}
//{{{
struct uniform_not_found_error : public std::logic_error {
  uniform_not_found_error (std::string const& errmsg) : logic_error(errmsg) { }
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
