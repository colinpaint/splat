//{{{  includes
#include "program.h"
#include "camera.h"

#include <iostream>
#include <cstdlib>

#include "../common/date.h"
#include "../common/cLog.h"

using namespace std;
//}}}
namespace {
  //{{{
  const vector<string> kMeshVsGlsl = {
    "#version 330",
    "#define SMOOTH 0",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "#define ATTR_POSITION 0",
    "layout(location = ATTR_POSITION) in vec3 position;",

    "#if SMOOTH",
      "#define ATTR_NORMAL 1",
      "layout(location = ATTR_NORMAL) in vec3 normal;",
    "#endif",

    "out block {",
      "#if SMOOTH",
        "vec3 normal;",
      "#endif",
      "vec3 position;",
      "}",
    "Out;",

    "void main() {",
      "vec4 position_eye = modelview_matrix * vec4(position, 1.0);",

      "#if SMOOTH",
        "Out.normal = mat3(modelview_matrix_it) * normal;",
      "#endif",

      "Out.position = vec3(position_eye);"
      "gl_Position = projection_matrix * position_eye;"
      "}"
    };
  //}}}
  //{{{
  const vector<string> kMeshGsGlsl = {
    "#version 330",
    "#define SMOOTH 0",
    "#define WIREFRAME 0",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "#if WIREFRAME",
      "layout(std140) uniform Wireframe {"
        "vec3 color_wireframe;"
        "ivec2 viewport;"
        "};",
    "#endif",

    "layout(triangles) in;",
    "layout(triangle_strip, max_vertices = 3) out;",

    "in block {",
      "#if SMOOTH",
        "vec3 normal;",
      "#endif",
      "vec3 position;",
      "}",
    "In[];",

    "out block {",
      "#if SMOOTH",
        "vec3 normal;",
      "#else",
        "flat vec3 normal;",
      "#endif",

      "vec3 position;",

      "#if WIREFRAME",
        "noperspective vec3 distance;",
      "#endif",
      "}",
    "Out;",

    "void main() {",
      "#if WIREFRAME",
        "vec2 w0 = (1.0 / gl_in[0].gl_Position.w) * gl_in[0].gl_Position.xy * viewport.xy;",
        "vec2 w1 = (1.0 / gl_in[1].gl_Position.w) * gl_in[1].gl_Position.xy * viewport.xy;",
        "vec2 w2 = (1.0 / gl_in[2].gl_Position.w) * gl_in[2].gl_Position.xy * viewport.xy;",
        "mat3 matA = mat3(vec3(1.0, w0), vec3(1.0, w1), vec3(1.0, w2));",
        "float area = abs(determinant(matA));",
      "#endif",

      "#if !SMOOTH",
        "vec3 normal = normalize(cross(In[1].position - In[0].position, In[2].position - In[0].position));",
      "#endif",

      "gl_Position = gl_in[0].gl_Position;",
      "#if SMOOTH",
        "Out.normal = In[0].normal;",
      "#else",
        "Out.normal = normal;",
      "#endif",
      "Out.position = In[0].position;",

      "#if WIREFRAME",
        "Out.distance = vec3(area / length(w2 - w1), 0.0, 0.0);",
      "#endif",
      "EmitVertex();",

      "gl_Position = gl_in[1].gl_Position;",
      "#if SMOOTH",
        "Out.normal = In[1].normal;",
      "#else",
        "Out.normal = normal;",
      "#endif",

      "Out.position = In[1].position;",
      "#if WIREFRAME",
        "Out.distance = vec3(0.0, area / length(w2 - w0), 0.0);",
      "#endif",
      "EmitVertex();",

      "gl_Position = gl_in[2].gl_Position;",
      "#if SMOOTH",
        "Out.normal = In[2].normal;",
      "#else",
        "Out.normal = normal;",
      "#endif",
      "Out.position = In[2].position;",
      "#if WIREFRAME",
        "Out.distance = vec3(0.0, 0.0, area / length(w1 - w0));",
      "#endif",
      "EmitVertex();",
      "}"
    };
  //}}}
  //{{{
  const vector<string> kMeshFsGlsl = {
    "#version 330",
    "#define SMOOTH 0",
    "#define WIREFRAME 0",

    "layout(std140) uniform Material {"
      "vec3 color;"
      "float shininess;"
      "}"
    "material;",

    "layout(std140) uniform Wireframe {"
      "vec3 color_wireframe;"
      "ivec2 viewport;"
      "};",

    "in block {",
      "#if SMOOTH",
        "vec3 normal;",
      "#else",
        "flat vec3 normal;",
      "#endif",

      "vec3 position;",

      "#if WIREFRAME",
        "noperspective vec3 distance;",
      "#endif",
      "}",
    "In;",

    "#define FRAG_COLOR 0",
    "layout(location = FRAG_COLOR, index = 0) out vec4 frag_color;",

    "void main() {",
      "#if SMOOTH",
        "vec3 normal_eye = normalize(In.normal);",
      "#else",
        "vec3 normal_eye = In.normal;",
      "#endif",

      "if (!gl_FrontFacing)"
        "normal_eye = -normal_eye;"

      "const vec3 light_eye = vec3(0.0, 0.0, 1.0);"

      "float dif = max(dot(light_eye, normal_eye), 0.0);"
      "vec3 view_eye = normalize(In.position);"
      "vec3 refl_eye = reflect(light_eye, normal_eye);"

      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), material.shininess);"
      "float rim = pow(1.0 + dot(normal_eye, view_eye), 3.0);"

      "vec3 color = 0.15 * material.color;"
      "color += 0.6 * dif * material.color;"
      "color += 0.1 * spe * vec3(1.0);"
      "color += 0.1 * rim * vec3(1.0);",

      "#if WIREFRAME",
        "float d = min(In.distance.x, min(In.distance.y, In.distance.z));",
        "float i = exp2(-0.75 * d * d);",
        "color = mix(color, color_wireframe, i);",
      "#endif",

      // Gamma correction. Assuming gamma of 2.0 rather than 2.2
      "frag_color = vec4(sqrt(color), 1.0);"
      "}"
    };
  //}}}
  //{{{
  const vector<string> kSphereVsGlsl = {
    "#version 330",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "layout(std140) uniform Sphere {"
      "float sphere_radius;"
      "float projection_radius;"
      "};",

    "#define ATTR_POSITION 0",
    "layout(location = ATTR_POSITION) in vec3 center;",

    "out block {"
      "flat vec3 center_eye;"
      "}"
    "Out;",

    "void main() {"
      "vec4 center_eye = modelview_matrix * vec4(center, 1.0);"
      "gl_Position = projection_matrix * center_eye;"
      "Out.center_eye = vec3(center_eye);"
      "gl_PointSize = 2.0 * (sphere_radius / -center_eye.z) * projection_radius;"
      "}"
    };
  //}}}
  //{{{
  const vector<string> kSphereFsGlsl = {
    "#version 330",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "layout(std140) uniform Sphere {"
      "float sphere_radius;"
      "float projection_radius;"
      "};",

    "layout(std140) uniform Material {"
      "vec3 color;"
      "float shininess;"
      "}"
    "material;",

    "in block {"
      "flat vec3 center_eye;"
      "}"
    "In;",

    "#define FRAG_COLOR 0",
    "layout(location = FRAG_COLOR, index = 0) out vec4 frag_color;",

    "void main() {"
      "vec2 p = gl_PointCoord * 2.0 - vec2(1.0);"
      "float z2 = 1.0 - p.x * p.x - p.y * p.y;"
      "if (z2 < 0.0)"
        "discard;"

      "float z = sqrt(z2);"
      "vec3 normal_eye = vec3(p.x, p.y, z);"
      "vec3 position_eye = In.center_eye + vec3(0.0, 0.0, sphere_radius * z);"
      "float depth = -projection_matrix[3][2] * (1.0 / position_eye.z) - projection_matrix[2][2];"
      "gl_FragDepth = (depth + 1.0) / 2.0;"
      "const vec3 light_eye = vec3(0.0, 0.0, 1.0);"
      "float dif = max(dot(light_eye, normal_eye), 0.0);"
      "vec3 view_eye = normalize(position_eye);"
      "vec3 refl_eye = reflect(light_eye, normal_eye);"
      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), material.shininess);"
      "float rim = pow(1.0 + dot(normal_eye, view_eye), 3.0);"

      "vec3 color = 0.15 * material.color;"
      "color += 0.6 * dif * material.color;"
      "color += 0.1 * spe * vec3(1.0);"
      "color += 0.1 * rim * vec3(1.0);"

      // Gamma correction. Assuming gamma of 2.0 rather than 2.2
      "frag_color = vec4(sqrt(color), 1.0);"
      "}"
    };
  //}}}
  }

namespace GLviz {
  // UniformBufferWireFrame
  //{{{
  void UniformBufferWireFrame::set_buffer_data (float const* color, int const* viewport) {

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), color);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLint), viewport);
    unbind();
    }
  //}}}

  // UniformBufferCamera
  UniformBufferCamera::UniformBufferCamera() : glUniformBuffer(48 * sizeof(GLfloat)) { }
  //{{{
  void UniformBufferCamera::set_buffer_data (Camera const& camera) {

    Eigen::Matrix4f const& modelview_matrix = camera.get_modelview_matrix();
    Eigen::Matrix4f modelview_matrix_it = modelview_matrix.inverse().transpose();
    Eigen::Matrix4f const& projection_matrix = camera.get_projection_matrix();

    bind();
    glBufferSubData (GL_UNIFORM_BUFFER, 0, 16 * sizeof(GLfloat), modelview_matrix.data());
    glBufferSubData (GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), 16 * sizeof(GLfloat), modelview_matrix_it.data());
    glBufferSubData (GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), 16 * sizeof(GLfloat), projection_matrix.data());
    unbind();
    }
  //}}}

  // UniformBufferMate\nrial
  //{{{
  void UniformBufferMaterial::set_buffer_data (float const* mbuf) {

    bind();
    glBufferData (GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), mbuf, GL_DYNAMIC_DRAW);
    unbind();
    }
  //}}}

  // UniformBufferWrireFrame
  UniformBufferWireFrame::UniformBufferWireFrame() : glUniformBuffer(4 * sizeof(GLfloat) + 2 * sizeof(GLint)) { }

  // UniformBufferSphere
  UniformBufferSphere::UniformBufferSphere() : glUniformBuffer(2 * sizeof(GLfloat)) { }
  //{{{
  void UniformBufferSphere::set_buffer_data (float radius, float projection) {

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat), &radius);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLfloat), sizeof(GLfloat), &projection);
    unbind();
    }
  //}}}

  // ProgramMesh3
  //{{{
  ProgramMesh3::ProgramMesh3() : glProgram(), m_wireFrame(false), m_smooth(false) {

    initShader();
    initProgram();
    }
  //}}}
  //{{{
  void ProgramMesh3::set_wireFrame (bool enable) {

    if (m_wireFrame != enable) {
      m_wireFrame = enable;
      initProgram();
      }
    }
  //}}}
  //{{{
  void ProgramMesh3::set_smooth (bool enable) {

    if (m_smooth != enable) {
      m_smooth = enable;
      initProgram();
      }
    }
  //}}}

  //{{{
  void ProgramMesh3::initShader() {

    m_mesh3_vs_obj.load (kMeshVsGlsl);
    m_mesh3_gs_obj.load (kMeshGsGlsl);
    m_mesh3_fs_obj.load (kMeshFsGlsl);

    attach_shader (m_mesh3_vs_obj);
    attach_shader (m_mesh3_gs_obj);
    attach_shader (m_mesh3_fs_obj);
    }
  //}}}
  //{{{
  void ProgramMesh3::initProgram() {

    try {
      map <string, int> defines;
      defines.insert (make_pair("WIREFRAME", m_wireFrame ? 1 : 0));
      defines.insert (make_pair("SMOOTH", m_smooth ? 1 : 0));

      m_mesh3_vs_obj.compile (defines);
      m_mesh3_gs_obj.compile (defines);
      m_mesh3_fs_obj.compile (defines);
      }
    catch (shader_compilation_error const& e) {
      cLog::log (LOGERROR, fmt::format ("ProgramMesh3::initProgram - failed compile {}", e.what()));
      exit(EXIT_FAILURE);
      }

    try {
      link();
      }
    catch (shader_link_error const& e) {
      cLog::log (LOGERROR, fmt::format ("ProgramMesh3::initProgram - failed link {}", e.what()));
      exit(EXIT_FAILURE);
      }

    try {
      setUniformBlockBind ("Camera", 0);
      setUniformBlockBind ("Material", 1);
      if (m_wireFrame)
        setUniformBlockBind ("Wireframe", 2);
      }
    catch (uniform_not_found_error const& e) {
      cLog::log (LOGERROR, fmt::format ("ProgramMesh3::initProgram - failed setUniformBlockBind {}", e.what()));
      }
    }
  //}}}

  // ProgramSphere
  //{{{
  ProgramSphere::ProgramSphere() {

    initShader();
    initProgram();
    }
  //}}}
  //{{{
  void ProgramSphere::initShader() {

    m_sphere_vs_obj.load (kSphereVsGlsl);
    m_sphere_fs_obj.load (kSphereFsGlsl);

    attach_shader (m_sphere_vs_obj);
    attach_shader (m_sphere_fs_obj);
    }
  //}}}
  //{{{
  void ProgramSphere::initProgram() {

    try {
      m_sphere_vs_obj.compile();
      m_sphere_fs_obj.compile();
      }
    catch (shader_compilation_error const& e) {
      cLog::log (LOGERROR, fmt::format ("ProgramSphere::initProgram - failed compile {}", e.what()));
      exit(EXIT_FAILURE);
      }

    try {
      link();
      }
    catch (shader_link_error const& e) {
      cLog::log (LOGERROR, fmt::format ("ProgramSphere::initProgram - failed link {}", e.what()));
      exit(EXIT_FAILURE);
      }

    try {
      setUniformBlockBind ("Camera", 0);
      setUniformBlockBind ("Material", 1);
      setUniformBlockBind ("Sphere", 3);
      }
    catch (uniform_not_found_error const& e) {
      cLog::log (LOGERROR, fmt::format ("ProgramSphere::initProgram - failed setUniformBlockBind {}", e.what()));
      }
    }
  //}}}
  }
