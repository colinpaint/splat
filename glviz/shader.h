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
class glShader {
public:
  virtual ~glShader();

  void load (const std::vector <std::string>& source);
  void loadFile (std::string const& filename);

  void compile (std::map<std::string, int> const& define_list = std::map<std::string, int>());
  bool isCompiled() const;

  std::string infolog();

protected:
  glShader();

  GLuint mShader;
  std::string m_source;

  friend class glProgram;
  };
//}}}
//{{{
class glVertexShader : public glShader {
public:
  glVertexShader();
  };
//}}}
//{{{
class glGeometryShader : public glShader {
public:
  glGeometryShader();
  };
//}}}
//{{{
class glFragmentShader : public glShader {
public:
  glFragmentShader();
  };
//}}}

//{{{
class glProgram {
public:
    glProgram();
    virtual ~glProgram();

    void use() const;
    void unuse() const;
    void link();

    void attach_shader(glShader& shader);
    void detach_shader(glShader& shader);

    void detach_all();

    bool is_linked();
    bool is_attached(glShader const& shader);
    std::string infolog();

    void setUniform1i (GLchar const* name, GLint value);
    void setUniformBlockBind (GLchar const* name, GLuint blockBind);

protected:
    GLuint mProgram;
  };
//}}}
