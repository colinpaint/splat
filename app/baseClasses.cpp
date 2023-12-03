//{{{  includes
#include "baseClasses.h"

#include <memory>
#include <sstream>
#include <fstream>
#include <cassert>

using namespace std;
//}}}

//{{{  cVertexArray
cVertexArray::cVertexArray() { glGenVertexArrays (1, &mVertexArray); }
cVertexArray::~cVertexArray() { glDeleteVertexArrays (1, &mVertexArray); }

void cVertexArray::bind() { glBindVertexArray (mVertexArray); }
void cVertexArray::unbind() { glBindVertexArray (0); }
//}}}
//{{{  cArray
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
//}}}
//{{{  cElementArray
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
//}}}
//{{{  cUniform
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
//}}}

//{{{  cShader
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
    throw fileOpenError ("Could not open file " + filename);

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
    throw shaderCompilationError (infoLog());
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
string cShader::infoLog() {

  GLint infoLogLength = 0;
  glGetShaderiv (mShader, GL_INFO_LOG_LENGTH, &infoLogLength);

  GLsizei logLength;
  unique_ptr<GLchar> infoLog (new GLchar[infoLogLength]);
  glGetShaderInfoLog (mShader, infoLogLength, &logLength, infoLog.get());

  return string (infoLog.get());
  }
//}}}
//}}}
cVertexShader::cVertexShader() { mShader = glCreateShader (GL_VERTEX_SHADER); }
cFragmentShader::cFragmentShader() { mShader = glCreateShader (GL_FRAGMENT_SHADER); }
cGeometryShader::cGeometryShader() { mShader = glCreateShader (GL_GEOMETRY_SHADER); }

//{{{  cProgram
cProgram::cProgram() : mProgram(glCreateProgram()) { }
//{{{
cProgram::~cProgram() {

  detachAll();
  glDeleteProgram (mProgram);
  }
//}}}

void cProgram::use() const { glUseProgram (mProgram); }
void cProgram::unuse() const { glUseProgram (0); }

//{{{
void cProgram::link() {

  glLinkProgram (mProgram);
  if (!isLinked())
    throw shaderLinkError (infoLog());
  }
//}}}

void cProgram::attachShader (cShader& shader) { glAttachShader (mProgram, shader.mShader); }
void cProgram::detachShader (cShader& shader) { glDetachShader (mProgram, shader.mShader); }
//{{{
void cProgram::detachAll() {

  GLsizei count;
  GLuint shader[64];
  glGetAttachedShaders (mProgram, 64, &count, shader);

  for (GLsizei i(0); i < count; ++i)
    glDetachShader (mProgram, shader[i]);
  }
//}}}

//{{{
bool cProgram::isLinked() {

  GLint status;
  glGetProgramiv (mProgram, GL_LINK_STATUS, &status);

  return (status == GL_TRUE);
  }
//}}}
//{{{
bool cProgram::isAttached (cShader const& shader) {

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
string cProgram::infoLog() {

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
    throw uniformNotFoundError (name);

  glProgramUniform1i (mProgram, location, value);
  }
//}}}
//{{{
void cProgram::setUniformBind (GLchar const* name, GLuint blockBind) {
// bind uniform block

  GLuint blockIndex = glGetUniformBlockIndex (mProgram, name);
  if (blockIndex == GL_INVALID_INDEX)
    throw uniformNotFoundError (name);

  glUniformBlockBinding (mProgram, blockIndex, blockBind);
  }
//}}}
//}}}
