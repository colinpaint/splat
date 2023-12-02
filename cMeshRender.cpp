//{{{  includes
#include "cMeshRender.h"

#include <Eigen/Core>

#include "../common/cLog.h"

#include "glviz/glviz.h"
#include "glviz/utility.h"

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
      "float shine;"
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

      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), material.shine);"
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
      "float shine;"
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
      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), material.shine);"
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

//{{{  cUniformMaterial
cUniformMaterial::cUniformMaterial() : cUniformBuffer (4 * sizeof(GLfloat)) {}

void cUniformMaterial::set (float const* mbuf) {
  bind();
  glBufferData (GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), mbuf, GL_DYNAMIC_DRAW);
  unbind();
  }
//}}}
//{{{  cUniformWireFrame
cUniformWireFrame::cUniformWireFrame()
  : cUniformBuffer (4 * sizeof(GLfloat) + 2 * sizeof(GLint)) {}

void cUniformWireFrame::set (float const* color, int const* viewport) {
  bind();
  glBufferSubData (GL_UNIFORM_BUFFER, 0, 4 * sizeof(GLfloat), color);
  glBufferSubData (GL_UNIFORM_BUFFER, 4 * sizeof(GLfloat), 2 * sizeof(GLint), viewport);
  unbind();
  }
//}}}
//{{{  cUniformSphere
cUniformSphere::cUniformSphere() : cUniformBuffer(2 * sizeof(GLfloat)) {}

void cUniformSphere::set (float radius, float projection) {
  bind();
  glBufferSubData (GL_UNIFORM_BUFFER, 0, sizeof(GLfloat), &radius);
  glBufferSubData (GL_UNIFORM_BUFFER, sizeof(GLfloat), sizeof(GLfloat), &projection);
  unbind();
  }
//}}}

//{{{  cProgramMesh
//{{{
cProgramMesh::cProgramMesh() : cProgram(), mWireFrame(false), mSmooth(false) {

  initShader();
  initProgram();
  }
//}}}

void cProgramMesh::setWireFrame (bool enable) {
  if (mWireFrame != enable) {
    mWireFrame = enable;
    initProgram();
    }
  }

void cProgramMesh::setSmooth (bool enable) {
  if (mSmooth != enable) {
    mSmooth = enable;
    initProgram();
    }
  }

void cProgramMesh::initShader() {
  mMeshVs.load (kMeshVsGlsl);
  mMeshGs.load (kMeshGsGlsl);
  mMeshFs.load (kMeshFsGlsl);
  attach_shader (mMeshVs);
  attach_shader (mMeshGs);
  attach_shader (mMeshFs);
  }

void cProgramMesh::initProgram() {
  try {
    map <string, int> defines;
    defines.insert (make_pair ("WIREFRAME", mWireFrame ? 1 : 0));
    defines.insert (make_pair ("SMOOTH", mSmooth ? 1 : 0));
    mMeshVs.compile (defines);
    mMeshGs.compile (defines);
    mMeshFs.compile (defines);
    }
  catch (shader_compilation_error const& e) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("ProgramMesh::initProgram - failed compile {}", e.what()));
    exit(EXIT_FAILURE);
    }
    //}}}

  try {
    link();
    }
  catch (shader_link_error const& e) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("ProgramMesh::initProgram - failed link {}", e.what()));
    exit(EXIT_FAILURE);
    }
    //}}}

  try {
    setUniformBind ("Camera", 0);
    setUniformBind ("Material", 1);
    if (mWireFrame)
      setUniformBind ("Wireframe", 2);
    }
  catch (uniform_not_found_error const& e) {
    //{{{  error
    cLog::log (LOGERROR, fmt::format ("ProgramMesh::initProgram - failed setUniformBind {}", e.what()));
    }
    //}}}
  }
//}}}
//{{{  cProgramSphere
cProgramSphere::cProgramSphere() {
  initShader();
  initProgram();
  }

void cProgramSphere::initShader() {
  mSphereVs.load (kSphereVsGlsl);
  mSphereFs.load (kSphereFsGlsl);
  attach_shader (mSphereVs);
  attach_shader (mSphereFs);
  }

void cProgramSphere::initProgram() {
  try {
    mSphereVs.compile();
    mSphereFs.compile();
    }
  catch (shader_compilation_error const& e) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("ProgramSphere::initProgram - failed compile {}", e.what()));
    exit(EXIT_FAILURE);
    }
    //}}}

  try {
    link();
    }
  catch (shader_link_error const& e) {
    //{{{  error, return
    cLog::log (LOGERROR, fmt::format ("ProgramSphere::initProgram - failed link {}", e.what()));
    exit(EXIT_FAILURE);
    }
    //}}}

  try {
    setUniformBind ("Camera", 0);
    setUniformBind ("Material", 1);
    setUniformBind ("Sphere", 3);
    }
  catch (uniform_not_found_error const& e) {
    //{{{  error
    cLog::log (LOGERROR, fmt::format ("ProgramSphere::initProgram - failed setUniformBind {}", e.what()));
    }
    //}}}
  }
//}}}

//{{{
cMeshRender::cMeshRender (GLviz::cCamera const& camera) : cRender(camera) {

  // setup vertex array v
  mVertexArrayV.bind();

  mVertexArrayBuffer.bind();
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));

  mVertexArrayV.unbind();

  // setup vertex array vf
  mVertexArrayVf.bind();
  mVertexArrayBuffer.bind();

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
  mIndexArrayBuffer.bind();

  mVertexArrayBuffer.unbind();
  mVertexArrayVf.unbind();

  // setup vertex array vnf
  mVertexArrayVnf.bind();
  mVertexArrayBuffer.bind();

  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
  mNormalArrayBuffer.bind();

  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<const GLvoid*>(0));
  mIndexArrayBuffer.bind();

  mVertexArrayBuffer.unbind();
  mVertexArrayVnf.unbind();

  use (mMultiSample, mBackFaceCull);
  }
//}}}
cMeshRender::~cMeshRender() {}

//{{{
void cMeshRender::use (bool multiSample, bool backFaceCull) {
// bind uniforms to binding points

  mMultiSample = multiSample;
  mBackFaceCull = backFaceCull;

  mUniformCamera.bindBufferBase (0);
  mUniformMaterial.bindBufferBase (1);
  mUniformWireFrame.bindBufferBase (2);
  mUniformWireSphere.bindBufferBase (3);
  }
//}}}

//{{{
void cMeshRender::gui() {

  if (ImGui::CollapsingHeader ("Mesh", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox ("draw", &mDisplayMesh);
    ImGui::Combo ("shading", &mShadingMethod, "flat\0phong\0\0");
    ImGui::ColorEdit3 ("color", mMeshMaterial);
    ImGui::DragFloat ("shine", &(mMeshMaterial[3]), 1e-2f, 1e-12f, 1000.0f);
    ImGui::Separator();

    ImGui::Checkbox ("wireFrame", &mDisplayWireFrame);
    ImGui::ColorEdit3 ("color##wire", mWireFrameMaterial);
    ImGui::Separator();
    }

 if (ImGui::CollapsingHeader ("Spheres", nullptr, ImGuiTreeNodeFlags_DefaultOpen)) {
   ImGui::Checkbox ("draw##spheres", &mDisplaySpheres);
   ImGui::DragFloat ("radius##spheres", &mPointRadius, 1e-5f, 0.0f, 0.1f, "%.4f");
   ImGui::ColorEdit3 ("color##spheres", mPointsMaterial);
   ImGui::DragFloat ("shine##spheres", &mPointsMaterial[3], 1e-2f, 1e-12f, 1000.0f);
   }
 }
//}}}
//{{{
bool cMeshRender::keyboard (SDL_Keycode key) {

  switch (key) {
    case SDLK_1: mDisplayMesh = !mDisplayMesh; return true;
    case SDLK_2: mDisplaySpheres = !mDisplaySpheres; return true;
    case SDLK_5: mShadingMethod = (mShadingMethod + 1) % 2; return true;
    case SDLK_w: mDisplayWireFrame = !mDisplayWireFrame; return true;
    }

  return false;
  }
//}}}
//{{{
void cMeshRender::display (cModel* model) {

  if (getMultiSample())
    glEnable (GL_MULTISAMPLE);

  glEnable (GL_DEPTH_TEST);

  glDepthMask (GL_TRUE);
  glClearDepth (1.0f);
  glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mVertexArrayBuffer.set (3 * model->getNumVertices() * sizeof(GLfloat), model->getVerticesData());
  mNormalArrayBuffer.set (3 * model->getNumNormals() * sizeof(GLfloat), model->getNormalsData());
  mIndexArrayBuffer.set (3 * model->getNumFaces() * sizeof(GLfloat), model->getFacesData());

  mUniformCamera.set (mCamera);

  if (mDisplayMesh) {
    mUniformMaterial.set (mMeshMaterial);

    array <int,2> screen = { GLviz::getScreenWidth(), GLviz::getScreenHeight() };
    mUniformWireFrame.set (mWireFrameMaterial, screen.data());

    //{{{  display mesh
    mProgramMesh.setWireFrame (mDisplayWireFrame);
    mProgramMesh.setSmooth (mShadingMethod != 0);

    mProgramMesh.use();

    if (mShadingMethod == 0) {
      // Flat
      mVertexArrayVf.bind();
      glDrawElements (GL_TRIANGLES, (GLsizei)model->getNumFaces() * 3,
                      GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
      mVertexArrayVf.unbind();
      }
    else {
      // Smooth
      mVertexArrayVnf.bind();
      glDrawElements (GL_TRIANGLES, (GLsizei)model->getNumFaces() * 3,
                      GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));
      mVertexArrayVnf.unbind();
      }

    mProgramMesh.unuse();
    //}}}
    }

  if (mDisplaySpheres) {
    mUniformMaterial.set (mPointsMaterial);

    GLviz::Frustum view_frustum = mCamera.get_frustum();
    mProjectionRadius =
      view_frustum.near_() * (GLviz::getScreenHeight() / (view_frustum.top() - view_frustum.bottom()));
    mUniformWireSphere.set (mPointRadius, mProjectionRadius);

    //{{{  display spheres
    glEnable (GL_PROGRAM_POINT_SIZE);
    glPointParameterf (GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    mProgramSphere.use();
    mVertexArrayV.bind();
    glDrawArrays (GL_POINTS, 0, static_cast<GLsizei>(model->getNumVertices()));
    mVertexArrayV.unbind();
    mProgramSphere.unuse();

    glDisable (GL_PROGRAM_POINT_SIZE);
    //}}}
    }

  glDisable (GL_DEPTH_TEST);

  if (getMultiSample())
    glDisable (GL_MULTISAMPLE);
  }
//}}}
