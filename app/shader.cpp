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

cShader::cShader() {}
cShader::~cShader() { glDeleteShader(mShader); }

//{{{
void cShader::load (const vector <string>& source) {

  m_source = "";
  for (auto& line : source)
    m_source += line + '\n';
  }
//}}}
//{{{
void cShader::loadFile (string const& filename) {

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
void cShader::compile (map<string, int> const& define_list) {

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
bool cShader::isCompiled() const {

  GLint status;
  glGetShaderiv (mShader, GL_COMPILE_STATUS, &status);

  return (status == GL_TRUE);
  }
//}}}

//{{{
string cShader::infolog() {

  GLint infoLogLength = 0;
  glGetShaderiv (mShader, GL_INFO_LOG_LENGTH, &infoLogLength);

  GLsizei logLength;
  unique_ptr<GLchar> infoLog (new GLchar[infoLogLength]);
  glGetShaderInfoLog (mShader, infoLogLength, &logLength, infoLog.get());

  return string (infoLog.get());
  }
//}}}

cVertexShader::cVertexShader() { mShader = glCreateShader (GL_VERTEX_SHADER); }
cFragmentShader::cFragmentShader() { mShader = glCreateShader (GL_FRAGMENT_SHADER); }
cGeometryShader::cGeometryShader() { mShader = glCreateShader (GL_GEOMETRY_SHADER); }

cProgram::cProgram() : mProgram(glCreateProgram()) { }
//{{{
cProgram::~cProgram() {

  detach_all();
  glDeleteProgram (mProgram);
  }
//}}}

void cProgram::use() const { glUseProgram(mProgram); }
void cProgram::unuse() const { glUseProgram(0); }

//{{{
void cProgram::link() {

  glLinkProgram (mProgram);
  if (!is_linked())
    throw shader_link_error (infolog());
  }
//}}}

void cProgram::attach_shader (cShader& shader) { glAttachShader (mProgram, shader.mShader); }
void cProgram::detach_shader (cShader& shader) { glDetachShader (mProgram, shader.mShader); }
//{{{
void cProgram::detach_all() {

  GLsizei count;
  GLuint shader[64];
  glGetAttachedShaders (mProgram, 64, &count, shader);

  for (GLsizei i(0); i < count; ++i)
    glDetachShader (mProgram, shader[i]);
  }
//}}}

//{{{
bool cProgram::is_linked() {

  GLint status;
  glGetProgramiv(mProgram, GL_LINK_STATUS, &status);

  return (status == GL_TRUE);
  }
//}}}
//{{{
bool cProgram::is_attached (cShader const& shader) {

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
string cProgram::infolog() {

  GLint infoLogLength = 0;
  glGetProgramiv (mProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

  GLsizei logLength;
  unique_ptr<GLchar> infoLog(new GLchar[infoLogLength]);
  glGetProgramInfoLog (mProgram, infoLogLength, &logLength, infoLog.get());

  return string (infoLog.get());
  }
//}}}

//{{{
void cProgram::setUniform1i (GLchar const* name, GLint value) {

  GLint location = glGetUniformLocation (mProgram, name);
  if (location == -1)
    throw uniform_not_found_error (name);

  glProgramUniform1i (mProgram, location, value);
  }
//}}}
//{{{
void cProgram::setUniformBind (GLchar const* name, GLuint blockBind) {
// bind uniform block

  GLuint blockIndex = glGetUniformBlockIndex (mProgram, name);
  if (blockIndex == GL_INVALID_INDEX)
    throw uniform_not_found_error (name);

  glUniformBlockBinding (mProgram, blockIndex, blockBind);
  }
//}}}
