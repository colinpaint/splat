//{{{  includes
#include "program.h"
#include "camera.h"

#include <Eigen/Core>
#include <iostream>
#include <cstdlib>

using namespace Eigen;

extern unsigned char const mesh3_vs_glsl[];
extern unsigned char const mesh3_gs_glsl[];
extern unsigned char const mesh3_fs_glsl[];

extern unsigned char const sphere_vs_glsl[];
extern unsigned char const sphere_fs_glsl[];

using namespace std;
//}}}
namespace {
  //{{{
  const std::string kSphereVsGlsl =
    "#version 330\n"

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};\n"

    "layout(std140) uniform Sphere {"
      "float sphere_radius;"
      "float projection_radius;"
      "};\n"

    "#define ATTR_POSITION 0\n"
    "layout(location = ATTR_POSITION) in vec3 center;\n"

    "out block {"
      "flat vec3 center_eye;"
      "}"
    "Out;\n"

    "void main() {"
      "vec4 center_eye = modelview_matrix * vec4(center, 1.0);"
      "gl_Position = projection_matrix * center_eye;"
      "Out.center_eye = vec3(center_eye);"
      "gl_PointSize = 2.0 * (sphere_radius / -center_eye.z) * projection_radius;"
      "}\n";
  //}}}
  //{{{
  const std::string kSphereFsGlsl =
    "#version 330\n"

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};\n"

    "layout(std140) uniform Sphere {"
      "float sphere_radius;"
      "float projection_radius;"
      "};\n"

    "layout(std140) uniform Material {"
      "vec3 color;"
      "float shininess;"
      "}"
    "material;\n"

    "in block {"
      "flat vec3 center_eye;"
      "}"
    "In;\n"

    "#define FRAG_COLOR 0\n"
    "layout(location = FRAG_COLOR, index = 0) out vec4 frag_color;\n"

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
      "}\n";
  //}}}
  //{{{
  const std::string kMeshGsGlsl =
    "#version 330\n"
    "#define SMOOTH 0\n"
    "#define WIREFRAME 0\n"

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};\n"

    "#if WIREFRAME\n"
      "layout(std140) uniform Wireframe {"
        "vec3 color_wireframe;"
        "ivec2 viewport;"
        "};\n"
    "#endif\n"

    "layout(triangles) in;\n"
    "layout(triangle_strip, max_vertices = 3) out;\n"

    "in block {\n"
      "#if SMOOTH\n"
        "vec3 normal;\n"
      "#endif\n"
      "vec3 position;"
      "}"
    "In[];\n"

    "out block {\n"
      "#if SMOOTH\n"
        "vec3 normal;\n"
      "#else\n"
        "flat vec3 normal;\n"
      "#endif\n"

      "vec3 position;\n"

      "#if WIREFRAME\n"
        "noperspective vec3 distance;\n"
      "#endif\n"
      "}"
    "Out;\n"

    "void main() {\n"
      "#if WIREFRAME\n"
        "vec2 w0 = (1.0 / gl_in[0].gl_Position.w) * gl_in[0].gl_Position.xy * viewport.xy;"
        "vec2 w1 = (1.0 / gl_in[1].gl_Position.w) * gl_in[1].gl_Position.xy * viewport.xy;"
        "vec2 w2 = (1.0 / gl_in[2].gl_Position.w) * gl_in[2].gl_Position.xy * viewport.xy;"
        "mat3 matA = mat3(vec3(1.0, w0), vec3(1.0, w1), vec3(1.0, w2));"
        "float area = abs(determinant(matA));\n"
      "#endif\n"

      "#if !SMOOTH\n"
        "vec3 normal = normalize(cross(In[1].position - In[0].position, In[2].position - In[0].position));\n"
      "#endif\n"

      "gl_Position = gl_in[0].gl_Position;\n"
      "#if SMOOTH\n"
        "Out.normal = In[0].normal;\n"
      "#else"
        "Out.normal = normal;\n"
      "#endif\n"
      "Out.position = In[0].position;\n"

      "#if WIREFRAME\n"
        "Out.distance = vec3(area / length(w2 - w1), 0.0, 0.0);\n"
      "#endif\n"
      "EmitVertex();"

      "gl_Position = gl_in[1].gl_Position;\n"
      "#if SMOOTH\n"
        "Out.normal = In[1].normal;\n"
      "#else\n"
        "Out.normal = normal;\n"
      "#endif\n"

      "Out.position = In[1].position;\n"
      "#if WIREFRAME\n"
        "Out.distance = vec3(0.0, area / length(w2 - w0), 0.0);\n"
      "#endif\n"
      "EmitVertex();"

      "gl_Position = gl_in[2].gl_Position;\n"
      "#if SMOOTH\n"
        "Out.normal = In[2].normal;\n"
      "#else\n"
        "Out.normal = normal;\n"
      "#endif\n"
      "Out.position = In[2].position;\n"
      "#if WIREFRAME\n"
        "Out.distance = vec3(0.0, 0.0, area / length(w1 - w0));\n"
      "#endif\n"
      "EmitVertex();"
      "}\n";
  //}}}
  //{{{
  const std::string kMeshVsGlsl =
    "#version 330\n"
    "#define SMOOTH 0\n"

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};\n"

    "#define ATTR_POSITION 0\n"
    "layout(location = ATTR_POSITION) in vec3 position;\n"

    "#if SMOOTH\n"
      "#define ATTR_NORMAL 1\n"
      "layout(location = ATTR_NORMAL) in vec3 normal;\n"
    "#endif\n"

    "out block {\n"
      "#if SMOOTH\n"
        "vec3 normal;\n"
      "#endif\n"
      "vec3 position;\n"
      "}\n"
    "Out;\n"

    "void main() {"
      "vec4 position_eye = modelview_matrix * vec4(position, 1.0);\n"

      "#if SMOOTH\n"
        "Out.normal = mat3(modelview_matrix_it) * normal;\n"
      "#endif\n"
      "Out.position = vec3(position_eye);"
      "gl_Position = projection_matrix * position_eye;"
      "}\n";
  //}}}
  //{{{
  const std::string kMeshFsGlsl =
    "#version 330\n"
    "#define SMOOTH     0\n"
    "#define WIREFRAME  0\n"

    "layout(std140) uniform Material {\n"
      "vec3 color;\n"
      "float shininess;\n"
      "}\n"
    "material;\n"

    "layout (std140) uniform Wireframe {\n"
      "vec3 color_wireframe;\n"
      "ivec2 viewport;\n"
      "};\n"

    "in block {\n"
      "#if SMOOTH\n"
        "vec3 normal;\n"
      "#else\n"
        "flat vec3 normal;\n"
      "#endif\n"

      "vec3 position;\n"

      "#if WIREFRAME\n"
        "noperspective vec3 distance;\n"
      "#endif\n"
      "}\n"
    "In;\n"

    "#define FRAG_COLOR 0\n"
    "layout(location = FRAG_COLOR, index = 0) out vec4 frag_color;\n"

    "void main() {\n"
      "#if SMOOTH\n"
        "vec3 normal_eye = normalize(In.normal);\n"
      "#else\n"
        "vec3 normal_eye = In.normal;\n"
      "#endif\n"

      "if (!gl_FrontFacing) {\n"
        "normal_eye = -normal_eye;\n"
        "}\n"

      "const vec3 light_eye = vec3(0.0, 0.0, 1.0);\n"

      "float dif = max(dot(light_eye, normal_eye), 0.0);\n"
      "vec3 view_eye = normalize(In.position);\n"
      "vec3 refl_eye = reflect(light_eye, normal_eye);\n"

      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), material.shininess);\n"
      "float rim = pow(1.0 + dot(normal_eye, view_eye), 3.0);\n"

      "vec3 color = 0.15 * material.color;\n"
      "color += 0.6 * dif * material.color;\n"
      "color += 0.1 * spe * vec3(1.0);\n"
      "color += 0.1 * rim * vec3(1.0);\n"

      "#if WIREFRAME\n"
        "float d = min(In.distance.x, min(In.distance.y, In.distance.z));\n"
        "float i = exp2(-0.75 * d * d);\n"
        "color = mix(color, color_wireframe, i);\n"
      "#endif\n"

      // Gamma correction. Assuming gamma of 2.0 rather than 2.2
      "frag_color = vec4(sqrt(color), 1.0);\n"
      "}\n";
  //}}}
  }

namespace GLviz {
  //{{{
  ProgramSphere::ProgramSphere()
  {
      initialize_shader_obj();
      initialize_program_obj();
  }
  //}}}
  //{{{
  void ProgramSphere::initialize_shader_obj() {

    m_sphere_vs_obj.load_from_string (kSphereVsGlsl);
    m_sphere_fs_obj.load_from_string (kSphereFsGlsl);

    attach_shader (m_sphere_vs_obj);
    attach_shader (m_sphere_fs_obj);
    }
  //}}}
  //{{{
  void ProgramSphere::initialize_program_obj() {

    try {
      m_sphere_vs_obj.compile();
      m_sphere_fs_obj.compile();
      }
    catch (shader_compilation_error const& e) {
      cerr << "Error: A shader failed to compile." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      link();
      }
    catch (shader_link_error const& e) {
      cerr << "Error: A program failed to link." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      set_uniform_block_binding ("Camera", 0);
      set_uniform_block_binding ("Material", 1);
      set_uniform_block_binding ("Sphere", 3);
      }
    catch (uniform_not_found_error const& e) {
      cerr << "Warning: Failed to set a uniform variable." << endl << e.what() << endl;
      }
    }
  //}}}

  //{{{
  void UniformBufferWireframe::set_buffer_data (float const* color, int const* viewport) {

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), color);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLint), viewport);
    unbind();
    }
  //}}}

  UniformBufferCamera::UniformBufferCamera() : glUniformBuffer(48 * sizeof(GLfloat)) { }
  //{{{
  void UniformBufferCamera::set_buffer_data (Camera const& camera) {

    Matrix4f const& modelview_matrix = camera.get_modelview_matrix();
    Matrix4f modelview_matrix_it = modelview_matrix.inverse().transpose();
    Matrix4f const& projection_matrix = camera.get_projection_matrix();

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(GLfloat), modelview_matrix.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(GLfloat), 16 * sizeof(GLfloat), modelview_matrix_it.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(GLfloat), 16 * sizeof(GLfloat), projection_matrix.data());
    unbind();
    }
  //}}}

  //{{{
  void UniformBufferMaterial::set_buffer_data (float const* mbuf) {

    bind();
    glBufferData (GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), mbuf, GL_DYNAMIC_DRAW);
    unbind();
    }
  //}}}
  UniformBufferWireframe::UniformBufferWireframe() : glUniformBuffer(4 * sizeof(GLfloat) + 2 * sizeof(GLint)) { }
  UniformBufferSphere::UniformBufferSphere() : glUniformBuffer(2 * sizeof(GLfloat)) { }
  //{{{
  void UniformBufferSphere::set_buffer_data (float radius, float projection) {

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLfloat), &radius);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(GLfloat), sizeof(GLfloat), &projection);
    unbind();
    }
  //}}}

  //{{{
  ProgramMesh3::ProgramMesh3() : glProgram(), m_wireframe(false), m_smooth(false) {

    initialize_shader_obj();
    initialize_program_obj();
    }
  //}}}
  //{{{
  void ProgramMesh3::set_wireframe (bool enable) {

    if (m_wireframe != enable) {
      m_wireframe = enable;
      initialize_program_obj();
      }
    }
  //}}}
  //{{{
  void ProgramMesh3::set_smooth (bool enable) {

    if (m_smooth != enable) {
      m_smooth = enable;
      initialize_program_obj();
      }
    }
  //}}}

  //{{{
  void ProgramMesh3::initialize_shader_obj() {

    m_mesh3_gs_obj.load_from_string (kMeshGsGlsl);
    m_mesh3_vs_obj.load_from_string (kMeshVsGlsl);
    m_mesh3_fs_obj.load_from_string (kMeshFsGlsl);

    attach_shader (m_mesh3_vs_obj);
    attach_shader (m_mesh3_gs_obj);
    attach_shader (m_mesh3_fs_obj);
    }
  //}}}
  //{{{
  void ProgramMesh3::initialize_program_obj() {

    try {
      map<string, int> defines;

      defines.insert(make_pair("WIREFRAME", m_wireframe ? 1 : 0));
      defines.insert(make_pair("SMOOTH", m_smooth ? 1 : 0));

      m_mesh3_vs_obj.compile(defines);
      m_mesh3_gs_obj.compile(defines);
      m_mesh3_fs_obj.compile(defines);
      }
    catch (shader_compilation_error const& e) {
      cerr << "Error: A shader failed to compile." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      link();
      }
    catch (shader_link_error const& e) {
      cerr << "Error: A program failed to link." << endl << e.what() << endl;
      exit(EXIT_FAILURE);
      }

    try {
      set_uniform_block_binding("Camera", 0);
      set_uniform_block_binding("Material", 1);

      if (m_wireframe)
        set_uniform_block_binding("Wireframe", 2);
      }
    catch (uniform_not_found_error const& e) {
      cerr << "Warning: Failed to set a uniform variable." << endl << e.what() << endl;
      }
    }
  //}}}
  }
