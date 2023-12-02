//{{{  includes
#include "shader.h"

#include <GL/glew.h>

#include <string>
#include <utility>
#include <fstream>
#include <sstream>
#include <memory>
#include <iostream>

using namespace std;
//}}}

glShader::glShader() {}
glShader::~glShader() { glDeleteShader(mShader); }

//{{{
void glShader::load (const vector <string>& source) {

  m_source = "";
  for (auto& line : source)
    m_source += line + '\n';
  }
//}}}
//{{{
void glShader::loadFile (string const& filename) {

  ifstream input (filename.c_str());
  if (input.fail())
    throw file_open_error ("Could not open file " + filename);

  ostringstream output;
  output << input.rdbuf();

  input.close();

  m_source = output.str();
  }
//}}}

//{{{
void glShader::compile (map<string, int> const& define_list) {

  // Configure source.
  string source = m_source;
  for (map<string, int>::const_iterator it = define_list.begin(); it != define_list.end(); ++it) {
    ostringstream define;
    define << "#define " << it->first;

    size_t pos = source.find (define.str(), 0);
    if (pos != string::npos) {
      size_t len = source.find ("\n", pos) - pos + 1;
      define << " " << it->second << "\n";
      source.replace (pos, len, define.str());
      }
    }

  // Compile configured source.
  const char* source_cstr = source.c_str();
  glShaderSource (mShader, 1, &source_cstr, NULL);
  glCompileShader (mShader);

  if (!isCompiled())
    throw shader_compilation_error (infolog());
  }
//}}}
//{{{
bool glShader::isCompiled() const {

  GLint status;
  glGetShaderiv (mShader, GL_COMPILE_STATUS, &status);

  return (status == GL_TRUE);
  }
//}}}

//{{{
string glShader::infolog() {

  GLint infoLogLength = 0;
  glGetShaderiv (mShader, GL_INFO_LOG_LENGTH, &infoLogLength);

  GLsizei logLength;
  unique_ptr<GLchar> infoLog (new GLchar[infoLogLength]);
  glGetShaderInfoLog (mShader, infoLogLength, &logLength, infoLog.get());

  return string (infoLog.get());
  }
//}}}

glVertexShader::glVertexShader() { mShader = glCreateShader(GL_VERTEX_SHADER); }
glFragmentShader::glFragmentShader() { mShader = glCreateShader(GL_FRAGMENT_SHADER); }
glGeometryShader::glGeometryShader() { mShader = glCreateShader(GL_GEOMETRY_SHADER); }

glProgram::glProgram() : mProgram(glCreateProgram()) { }
//{{{
glProgram::~glProgram() {

  detach_all();
  glDeleteProgram (mProgram);
  }
//}}}

void glProgram::use() const { glUseProgram(mProgram); }
void glProgram::unuse() const { glUseProgram(0); }

//{{{
void glProgram::link() {

  glLinkProgram (mProgram);
  if (!is_linked())
    throw shader_link_error (infolog());
  }
//}}}

void glProgram::attach_shader (glShader& shader) { glAttachShader (mProgram, shader.mShader); }
void glProgram::detach_shader (glShader& shader) { glDetachShader (mProgram, shader.mShader); }
//{{{
void glProgram::detach_all() {

  GLsizei count;
  GLuint shader[64];
  glGetAttachedShaders (mProgram, 64, &count, shader);

  for (GLsizei i(0); i < count; ++i)
    glDetachShader (mProgram, shader[i]);
  }
//}}}

//{{{
bool glProgram::is_linked() {

  GLint status;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &status);

  return (status == GL_TRUE);
  }
//}}}
//{{{
bool glProgram::is_attached (glShader const& shader) {

  GLint number_shader_attached;
  glGetProgramiv (mProgram, GL_ATTACHED_SHADERS, &number_shader_attached);

  unique_ptr<GLuint> shader_list (new GLuint[number_shader_attached]);
  GLsizei count;
  glGetAttachedShaders (mProgram, static_cast<GLsizei>(number_shader_attached), &count, shader_list.get());

  for (unsigned int i = 0; i < static_cast<GLuint>(count); ++i)
    if ((shader_list.get())[i] == shader.mShader)
      return true;

  return false;
  }
//}}}

//{{{
string glProgram::infolog() {

  GLint infoLogLength = 0;
  glGetProgramiv (mProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

  GLsizei logLength;
  unique_ptr<GLchar> infoLog(new GLchar[infoLogLength]);
  glGetProgramInfoLog (mProgram, infoLogLength, &logLength, infoLog.get());

  return string (infoLog.get());
  }
//}}}

//{{{
void glProgram::setUniform1i (GLchar const* name, GLint value) {

  GLint location = glGetUniformLocation (mProgram, name);
  if (location == -1)
    throw uniform_not_found_error (name);

  glProgramUniform1i (mProgram, location, value);
  }
//}}}
//{{{
void glProgram::setUniformBind (GLchar const* name, GLuint blockBind) {
// bind uniform block

  GLuint blockIndex = glGetUniformBlockIndex (mProgram, name);
  if (blockIndex == GL_INVALID_INDEX)
    throw uniform_not_found_error (name);

  glUniformBlockBinding (mProgram, blockIndex, blockBind);
  }
//}}}
