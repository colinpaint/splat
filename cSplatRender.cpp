//{{{  includes
#include "cSplatRender.h"

#include "../common/cLog.h"

#include "glviz/glviz.h"
#include "glviz/utility.h"

using namespace std;
//}}}
namespace {
  //{{{
  const vector<string> kAttributeVsGlsl = {
    "#version 330",
    "#define VISIBILITY_PASS 0",
    "#define BACKFACE_CULLING 0",
    "#define SMOOTH 0",
    "#define COLOR_MATERIAL 0",
    "#define EWA_FILTER 0",
    "#define POINTSIZE_METHOD 0",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "layout(std140, column_major) uniform Raycast {"
      "mat4 projection_matrix_inv;"
      "vec4 viewport;"
      "};",

    "layout(std140) uniform Frustum {"
      "vec4 frustum_plane[6];"
      "};",

    "layout(std140) uniform Parameter {"
      "vec3 material_color;"
      "float material_shininess;"
      "float radius_scale;"
      "float ewa_radius;"
      "float epsilon;"
      "};",

    "#define ATTR_CENTER 0",
    "layout(location = ATTR_CENTER) in vec3 c;",
    "#define ATTR_T1 1",
    "layout(location = ATTR_T1) in vec3 u;",
    "#define ATTR_T2 2",
    "layout(location = ATTR_T2) in vec3 v;",
    "#define ATTR_PLANE 3",
    "layout(location = ATTR_PLANE) in vec3 p;",
    "#define ATTR_COLOR 4",
    "layout(location = ATTR_COLOR) in vec4 rgba;",

    //{{{
    "out block {",
      "flat out vec3 c_eye;",
      "flat out vec3 u_eye;",
      "flat out vec3 v_eye;",
      "flat out vec3 p;",
      "flat out vec3 n_eye;",

      "#if !VISIBILITY_PASS",
        "#if EWA_FILTER",
          "flat out vec2 c_scr;",
        "#endif",
        "flat out vec3 color;",
      "#endif",
      "}",
    "Out;",
    //}}}
    "#if !VISIBILITY_PASS",
      "vec3 lighting (vec3 n_eye, vec3 v_eye, vec3 color, float shininess);",
    "#endif",

    //{{{
    "void intersect (in vec4 v1, in vec4 v2, in int p, out int n_pts, out vec4[2] pts) {"
      "int i = p / 2;"
      "float j = float(-1 + 2 * (p % 2));"

      "float b1 = v1.w + float(j) * v1[i];"
      "float b2 = v2.w + float(j) * v2[i];"

      "bool tb1 = b1 > 0.0;"
      "bool tb2 = b2 > 0.0;"

      "n_pts = 0;"

      "if (tb1 && tb2) {"
        "pts[0] = v2;"
        "n_pts = 1;"
        "}"
      "else if (tb1 && !tb2) {"
        "float a = b1 / (b1 - b2);"
        "pts[0] = (1.0 - a) * v1 + a * v2;"
        "n_pts = 1;"
        "}"
      "else if (!tb1 && tb2) {"
        "float a = b1 / (b1 - b2);"
        "pts[0] = (1.0 - a) * v1 + a * v2;"
        "pts[1] = v2;"
        "n_pts = 2;"
        "}"
      "}",
    //}}}
    //{{{
    "void clip_polygon (in vec4 p0[4], out int n_pts, out vec4 p1[8]) {"
      "vec4 p[8];"
      "int n = 4;"

      "p[0] = p0[0];"
      "p[1] = p0[1];"
      "p[2] = p0[2];"
      "p[3] = p0[3];"

      "for (int i = 0; i < 6; ++i) {"
        "int k = 0;"

        "for (int j = 0; j < n; ++j) {"
          "int n_pts;"
          "vec4 pts[2];"
          "intersect(p[j], p[(j + 1) % n], i, n_pts, pts);"

          "if (n_pts == 1) {"
            "p1[k++] = pts[0];"
            "}"
          "else if (n_pts == 2) {"
            "p1[k++] = pts[0];"
            "p1[k++] = pts[1];"
            "}"
          "}"

        "for (int j = 0; j < k; ++j) {"
         "p[j] = p1[j];"
          "}"

        "n = k;"
        "}"

      "n_pts = n;"
      "}",
    //}}}
    //{{{
    "void conic_Q (in vec3 u, in vec3 v, in vec3 c, out mat3 Q1) {"
    "             mat3 Q0 = mat3(vec3(1.0, 0.0, 0.0),"
    "             vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, -1.0));"

      "mat3 Sinv = transpose(mat3(cross(v, c), cross(u, c), cross(u, v)));"
      "Sinv[0][1] = -Sinv[0][1];"
      "Sinv[1][1] = -Sinv[1][1];"
      "Sinv[2][1] = -Sinv[2][1];"

      "mat3 Pinv = mat3 (vec3(projection_matrix_inv[0]),"
                         "vec3(projection_matrix_inv[1]),"
                         "vec3(projection_matrix_inv[2]));"
      "Pinv[2][2] = -1.0;"

      "mat3 Minv = Sinv * Pinv;"
      "Q1 = transpose(Minv) * Q0 * Minv;"
      "}",
    //}}}
    //{{{
    "void pointsprite (in vec3 c, in vec3 u, in vec3 v, out vec4 p_scr, out vec2 w) {",
      "#if POINTSIZE_METHOD == 0",
        // This method obtains the position and bounds of a splat by
        // clipping and perspectively projecting a bounding polygon.
        "mat4 M = projection_matrix * mat4("
            "vec4(u, 0.0), vec4(v, 0.0), vec4(c, 0.0),"
            "vec4(0.0, 0.0, 0.0, 1.0));"

        "vec4 p0[4];"
        "p0[0] = M * vec4(1.0);"
        "p0[1] = M * vec4(1.0, -1.0, 1.0, 1.0);"
        "p0[2] = M * vec4(-1.0, -1.0, 1.0, 1.0);"
        "p0[3] = M * vec4(-1.0, 1.0, 1.0, 1.0);"

        "int n_pts;"
        "vec4 p1[8];"

        "clip_polygon (p0, n_pts, p1);"

        "if (n_pts == 0) {"
          "p_scr = vec4(1.0, 0.0, 0.0, 0.0);"
          "w = vec2(0.0);"
          "}"
        "else {"
          "vec2 p_min = vec2(1.0);"
          "vec2 p_max = vec2(-1.0);"

          "for (int i = 0; i < n_pts; ++i) {"
            "vec2 p1i = p1[i].xy / p1[i].w;"
            "p_min = min(p_min, p1i);"
            "p_max = max(p_max, p1i);"
            "}"

          "w = 0.5 * (p_max - p_min);"

          "p_scr.xy = p_min + w;"
          "p_scr.z = 0.0;"
          "p_scr.w = 1.0;"
          "}",
      //{{{
      "#elif POINTSIZE_METHOD == 1",
        "p_scr = projection_matrix * vec4(c, 1.0);"
        "float p11 = projection_matrix[1][1];"

        "float r = max(length(u), length(v));"
        "w = vec2(0.0, r * p11 / abs(c.z));",
      //}}}
      //{{{
      "#elif POINTSIZE_METHOD == 2",
        // WHA+07.
        "float r = max(length(u), length(v));"

        "vec3 pl = vec3("
        "    dot(frustum_plane[0], vec4(c, 1.0)),"
        "    dot(frustum_plane[2], vec4(c, 1.0)),"
        "    dot(frustum_plane[4], vec4(c, 1.0)));"

        "vec3 pr = vec3("
        "    dot(frustum_plane[1], vec4(c, 1.0)),"
        "    dot(frustum_plane[3], vec4(c, 1.0)),"
        "    dot(frustum_plane[5], vec4(c, 1.0)));"

        "bool t_lr = (pl.x + r) > 0.0 && (pr.x + r) > 0.0;"
        "bool t_bt = (pl.y + r) > 0.0 && (pr.y + r) > 0.0;"
        "bool t_nf = (pl.z + r) > 0.0 && (pr.z + r) > 0.0;"

        "if (t_lr && t_bt && t_nf) {"
          "mat4x3 T = transpose(projection_matrix * mat3x4(vec4(u, 0.0), vec4(v, 0.0), vec4(c, 1.0) ));"

          "float d = dot(vec3(1.0, 1.0, -1.0), T[3] * T[3]);"
          "vec3 f = (1.0 / d) * vec3(1.0, 1.0, -1.0);"

          "vec3 p = vec3(dot(f, T[0] * T[3]), dot(f, T[1] * T[3]), dot(f, T[2] * T[3]));"

          "vec3 h0 = p * p - vec3(dot(f, T[0] * T[0]), dot(f, T[1] * T[1]), dot(f, T[2] * T[2]));"
          "vec3 h = sqrt(max(vec3(0.0), h0)) + vec3(0.0, 0.0, 1e-2);"

          "w = h.xy;"
          "p_scr = vec4(p.xy, 0.0, 1.0);"
          "}"
        "else {"
          "p_scr = vec4(1.0, 0.0, 0.0, 0.0);"
          "w = vec2(0.0);"
        "}",
      //}}}
      //{{{
      "#elif POINTSIZE_METHOD == 3",
        // ZRB+04.
        "mat3 Q;"
        "conic_Q(u, v, c, Q);"

        "float Qa = Q[0][0]; float Qb = Q[1][0];"
        "float Qc = Q[1][1]; float Qd = Q[2][0];"
        "float Qe = Q[2][1]; float Qf = -Q[2][2];"

        "float delta = Qa * Qc - Qb * Qb;"

        "if (delta > 0.0) {"
          "vec2 p = (Qb * vec2(Qe, Qd) - vec2(Qc * Qd, Qa * Qe)) / delta;"
          "float bb = Qf - dot(vec2(Qd, Qe), p);"

          "mat3 Q2 = mat3(vec3(Qa, Qb, 0.0),"
                         "vec3(Qb, Qc, 0.0),"
                         "vec3(0.0, 0.0, -bb)) / bb;"

          "float delta2 = Q2[0][0] * Q2[1][1] - Q2[0][1] * Q2[0][1];"
          "vec2 h = sqrt(vec2(Q2[1][1], Q2[0][0]) / delta2);"

          // Put an upper bound on the point size since the
          // centralized conics method is numerically unstable.
          "w = clamp(h, 0.0, 0.1);"
          "p_scr = vec4(p, 0.0, 1.0);"
          "}"
        "else {"
          "p_scr = vec4(1.0, 0.0, 0.0, 0.0);"
          "w = vec2(0.0);"
          "}",
      "#endif",
      //}}}
      "}",
    //}}}

    //{{{
    "void main() {",
      "vec4 c_eye = modelview_matrix * vec4(c, 1.0);",
      "vec3 u_eye = radius_scale * mat3(modelview_matrix) * u;",
      "vec3 v_eye = radius_scale * mat3(modelview_matrix) * v;",
      "vec3 n_eye = normalize(cross(u_eye, v_eye));",

      "vec4 p_scr;",
      "vec2 w;",
      "pointsprite (c_eye.xyz, u_eye, v_eye, p_scr, w);",

      "#if !VISIBILITY_PASS",
        "#if SMOOTH",
          "#if COLOR_MATERIAL",
            "Out.color = material_color;",
          "#else",
            "Out.color = vec3(rgba);",
          "#endif",
        "#else",
          "#if COLOR_MATERIAL",
            "Out.color = lighting (n_eye, vec3(c_eye), material_color, material_shininess);",
          "#else",
            "Out.color = lighting (n_eye, vec3(c_eye), vec3(rgba), material_shininess);",
          "#endif",
        "#endif",
      "#endif",

      "#if BACKFACE_CULLING",
        // Backface culling
        "if (dot(n_eye, -vec3(c_eye)) > 0.0) {",
      "#endif",
          // Pointsprite position"
          "gl_Position = p_scr;",

          "Out.c_eye = vec3(c_eye);",
          "Out.u_eye = u_eye;",
          "Out.v_eye = v_eye;",
          "Out.p = p;",
          "Out.n_eye = n_eye;",

          // Pointsprite size. One additional pixel avoids artifacts.
          "float point_size = max(w[0] * viewport.z, w[1] * viewport.w) + 1.0;",

      "#if !VISIBILITY_PASS && EWA_FILTER",
        "Out.c_scr = vec2((p_scr.xy + 1.0) * viewport.zw * 0.5);",
        "gl_PointSize = max(2.0, point_size);",
      "#else",
        "gl_PointSize = point_size;",
      "#endif",

      "#if BACKFACE_CULLING",
        "}",
      "else {",
        "gl_Position = vec4(1.0, 0.0, 0.0, 0.0);",
        "}",
      "#endif",
      "}",
    //}}}
    };
  //}}}
  //{{{
  const vector<string> kAttributeFsGlsl = {
    "#version 330",
    "#define VISIBILITY_PASS 0",
    "#define SMOOTH 0",
    "#define EWA_FILTER 0",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "layout(std140, column_major) uniform Raycast {"
      "mat4 projection_matrix_inv;"
      "vec4 viewport;"
      "};",

    "layout(std140) uniform Frustum {"
      "vec4 frustum_plane[6];"
      "};",

    "layout(std140) uniform Parameter {"
      "vec3 material_color;"
      "float material_shininess;"
      "float radius_scale;"
      "float ewa_radius;"
      "float epsilon;"
      "};",

    "uniform sampler1D filter_kernel;",

    "in block {"
      "flat in vec3 c_eye;"
      "flat in vec3 u_eye;"
      "flat in vec3 v_eye;"
      "flat in vec3 p;"
      "flat in vec3 n_eye;",

      "#if !VISIBILITY_PASS",
        "#if EWA_FILTER",
          "flat in vec2 c_scr;",
        "#endif",
      "flat in vec3 color;",
      "#endif",
      "}",
    "In;",

    "#define FRAG_COLOR 0",
    "layout(location = FRAG_COLOR) out vec4 frag_color;",

    "#if !VISIBILITY_PASS",
      "#if SMOOTH",
        "#define FRAG_NORMAL 1",
          "layout(location = FRAG_NORMAL) out vec4 frag_normal;",
      "#endif",
    "#endif",

    //{{{
    "void main() {",
      "vec4 p_ndc = vec4(2.0 * (gl_FragCoord.xy - viewport.xy) / (viewport.zw) - 1.0, -1.0, 1.0);",
      "vec4 p_eye = projection_matrix_inv * p_ndc;",
      "vec3 qn = p_eye.xyz / p_eye.w;",
      "vec3 q = qn * dot(In.c_eye, In.n_eye) / dot(qn, In.n_eye);",
      "vec3 d = q - In.c_eye;",

      "vec2 u = vec2(dot(In.u_eye, d) / dot(In.u_eye, In.u_eye),",
                    "dot(In.v_eye, d) / dot(In.v_eye, In.v_eye));",

      "if (dot(vec3(u, 1.0), In.p) < 0)",
        "discard;",

      "float w3d = length(u);",
      "float zval = q.z;",

      "#if !VISIBILITY_PASS && EWA_FILTER",
        "float w2d = distance(gl_FragCoord.xy, In.c_scr) / ewa_radius;",
        "float dist = min(w2d, w3d);",

        // Avoid visual artifacts due to wrong z-values for fragments
        // being part of the low-pass filter, but outside of the reconstruction filter.
        "if (w3d > 1.0)",
          "zval = In.c_eye.z;",
      "#else",
        "float dist = w3d;",
      "#endif",

      "if (dist > 1.0)",
        "discard;",

      "#if !VISIBILITY_PASS",
        "#if EWA_FILTER",
          "float alpha = texture(filter_kernel, dist).r;",
        "#else",
          "float alpha = 1.0;",
        "#endif",

        "frag_color = vec4(In.color, alpha);",

        "#if SMOOTH",
          "frag_normal = vec4(In.n_eye, alpha);",
        "#endif",
      "#endif",

      "#if VISIBILITY_PASS",
        "zval -= epsilon;",
      "#endif",

      "float depth = -projection_matrix[3][2] * (1.0 / zval) - projection_matrix[2][2];",

      "gl_FragDepth = (depth + 1.0) / 2.0;",
      "}",
    //}}}
    };
  //}}}
  //{{{
  const vector<string> kFinalVsGlsl = {
    "#version 330",

    "#define ATTR_POSITION 0",
    "layout(location = ATTR_POSITION) in vec3 position;",

    "#define ATTR_TEXTURE_UV 1",
    "layout(location = ATTR_TEXTURE_UV) in vec2 texture_uv;",

    "out block {"
      "vec2 texture_uv;"
      "}"
    "Out;",

    "void main() {"
      "gl_Position = vec4(position, 1.0);"
      "Out.texture_uv = texture_uv;"
      "}"
    };
  //}}}
  //{{{
  const vector<string> kFinalFsGlsl = {
    "#version 330",
    "#define MULTISAMPLING 0",
    "#define SMOOTH 0",

    "layout(std140, column_major) uniform Camera {"
      "mat4 modelview_matrix;"
      "mat4 modelview_matrix_it;"
      "mat4 projection_matrix;"
      "};",

    "layout(std140, column_major) uniform Raycast {"
      "mat4 projection_matrix_inv;"
      "vec4 viewport;"
      "};",

    "layout(std140) uniform Frustum {"
      "vec4 frustum_plane[6];"
      "};",

    "layout(std140) uniform Parameter {"
      "vec3 material_color;"
      "float material_shininess;"
      "float radius_scale;"
      "float ewa_radius;"
      "float epsilon;"
      "};",

    "#if MULTISAMPLING",
      "uniform sampler2DMS color_texture;",
    "#else",
      "uniform sampler2D color_texture;",
    "#endif",

    "#if SMOOTH",
      "#if MULTISAMPLING",
        "uniform sampler2DMS normal_texture;",
        "uniform sampler2DMS depth_texture;",
      "#else",
        "uniform sampler2D normal_texture;",
        "uniform sampler2D depth_texture;",
      "#endif",

      "vec3 lighting (vec3 n_eye, vec3 v_eye, vec3 color, float shininess);",
    "#endif",

    "in block {"
      "vec2 texture_uv;"
      "}"
    "In;",

    "#define FRAG_COLOR 0",
    "layout(location = FRAG_COLOR) out vec4 frag_color;",

    //{{{
    "void main() {",
      "vec4 res = vec4(0.0);",

      "#if MULTISAMPLING",
        "ivec2 itexture_uv = ivec2(textureSize(color_texture) * In.texture_uv);",
        "for (int i = 0; i < 4; ++i)",
      "#endif",
          "{",
          "#if MULTISAMPLING",
            "vec4 pixel = texelFetch(color_texture, itexture_uv, i);",

            "#if SMOOTH",
              "vec3 normal = normalize(texelFetch(normal_texture, ivec2(itexture_uv), i).xyz);",
              "float depth = texelFetch(depth_texture, ivec2(itexture_uv), i).r;",
            "#endif",
          "#else",
            "vec4 pixel = texture(color_texture, In.texture_uv);",

            "#if SMOOTH",
              "vec3 normal = normalize(texture(normal_texture, In.texture_uv).xyz);",
              "float depth = texture(depth_texture, In.texture_uv).r;",
            "#endif",
          "#endif",

          "if (pixel.a > 0.0) {",
            "#if SMOOTH",
              "vec4 p_ndc = vec4 (2.0 * (gl_FragCoord.xy - viewport.xy) / (viewport.zw) - 1.0,",
                                "(2.0 * depth - gl_DepthRange.near - gl_DepthRange.far) / gl_DepthRange.diff,",
                                "1.0 );"
              "vec4 v_eye = projection_matrix_inv * p_ndc;"
              "v_eye = v_eye / v_eye.w;"
              "res += vec4(lighting(normal, v_eye.xyz, pixel.rgb / pixel.a, material_shininess), 1.0);",
            "#else",
               "res += vec4(pixel.rgb / pixel.a, 1.0f);",
            "#endif",
            "}",
          "else {"
            "res += vec4(1.0);"
            "}"
          "}",

      "#if MULTISAMPLING",
        "frag_color = sqrt(res / 4.0);",
      "#else",
        "frag_color = sqrt(res);",
      "#endif",
      "}"
    //}}}
    };
  //}}}
  //{{{
  const vector<string> kLightingGlsl = {
    "#version 330",

    "vec3 lighting (vec3 normal_eye, vec3 v_eye, vec3 color, float shininess) {"
      "const vec3 light_eye = vec3(0.0, 0.0, 1.0);"
      "float dif = max(dot(light_eye, normal_eye), 0.0);"
      "vec3 refl_eye = reflect(light_eye, normal_eye);"
      "vec3 view_eye = normalize(v_eye);"

      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), shininess);"
      "float rim = pow(1.0 + dot(normal_eye, view_eye), 3.0);"

      "vec3 res = 0.15 * color;"
      "res += 0.6 * dif * color;"
      "res += 0.1 * spe * vec3(1.0);"
      "res += 0.1 * rim * vec3(1.0);"
      "return res;"
      "}"
    };
  //}}}
  }

//{{{  UniformBufferRaycast
UniformBufferRaycast::UniformBufferRaycast() : glUniformBuffer(sizeof(Eigen::Matrix4f) + sizeof(Eigen::Vector4f)) { }
//{{{
void UniformBufferRaycast::set_buffer_data (Eigen::Matrix4f const&
                                            projection_matrix_inv, GLint const* viewport) {

  float viewportf[4] = { static_cast<float>(viewport[0]),
                         static_cast<float>(viewport[1]),
                         static_cast<float>(viewport[2]),
                         static_cast<float>(viewport[3]) };
  bind();
  glBufferSubData (GL_UNIFORM_BUFFER, 0, sizeof(Eigen::Matrix4f), projection_matrix_inv.data());
  glBufferSubData (GL_UNIFORM_BUFFER, sizeof(Eigen::Matrix4f), 4 * sizeof(float), viewportf);
  unbind();
  }
//}}}
//}}}
//{{{  UniformBufferFrustum
UniformBufferFrustum::UniformBufferFrustum() : glUniformBuffer(6 * sizeof(Eigen::Vector4f)) { }
//{{{
void UniformBufferFrustum::set_buffer_data (Eigen::Vector4f const* frustum_plane) {

  bind();
  glBufferSubData(GL_UNIFORM_BUFFER, 0, 6 * sizeof(Eigen::Vector4f), static_cast<void const*>(frustum_plane));
  unbind();
  }
//}}}
//}}}
//{{{  UniformBufferParameter
UniformBufferParameter::UniformBufferParameter() : glUniformBuffer(8 * sizeof(float)) { }
//{{{
void UniformBufferParameter::set_buffer_data (Eigen::Vector3f const& color, float shininess,
                                              float radius_scale, float ewa_radius, float epsilon) {

  bind();
  glBufferSubData (GL_UNIFORM_BUFFER, 0, 3 * sizeof(float), color.data());
  glBufferSubData (GL_UNIFORM_BUFFER, 12, sizeof(float), &shininess);
  glBufferSubData (GL_UNIFORM_BUFFER, 16, sizeof(float), &radius_scale);
  glBufferSubData (GL_UNIFORM_BUFFER, 20, sizeof(float), &ewa_radius);
  glBufferSubData (GL_UNIFORM_BUFFER, 24, sizeof(float), &epsilon);
  unbind();
  }
//}}}
//}}}
//{{{  Framebuffer
//{{{
struct Framebuffer::Impl {
  virtual bool multisample() const = 0;

  virtual void framebuffer_texture_2d (GLenum target, GLenum attachment, GLuint texture, GLint level) = 0;
  virtual void renderbuffer_storage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = 0;

  virtual void allocate_depth_texture (GLuint texture, GLsizei width, GLsizei height) = 0;
  virtual void allocate_rgba_texture (GLuint texture, GLsizei width, GLsizei height) = 0;

  virtual void resize_rgba_texture (GLuint texture, GLsizei width, GLsizei height) = 0;
  virtual void resize_depth_texture (GLuint texture, GLsizei width, GLsizei height) = 0;
  };
//}}}
//{{{
struct Framebuffer::Default : public Framebuffer::Impl {
  bool multisample() const { return false; }

  //{{{
  void framebuffer_texture_2d (GLenum target, GLenum attachment, GLuint texture, GLint level) {
    glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texture, level);
    }
  //}}}
  //{{{
  void renderbuffer_storage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    glRenderbufferStorage(target, internalformat, width, height);
    }
  //}}}

  //{{{
  void allocate_depth_texture (GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture (GL_TEXTURE_2D, texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glBindTexture (GL_TEXTURE_2D, 0);
    }
  //}}}
  //{{{
  void allocate_rgba_texture (GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture (GL_TEXTURE_2D, texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture (GL_TEXTURE_2D, 0);
    }
  //}}}

  //{{{
  void resize_rgba_texture (GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture (GL_TEXTURE_2D, texture);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture (GL_TEXTURE_2D, 0);
    }
  //}}}
  //{{{
  void resize_depth_texture (GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture (GL_TEXTURE_2D, texture);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glBindTexture (GL_TEXTURE_2D, 0);
    }
  //}}}
  };
//}}}
//{{{
struct Framebuffer::Multisample : public Framebuffer::Impl {
  bool multisample() const { return true; }

  //{{{
  void framebuffer_texture_2d(GLenum target, GLenum attachment, GLuint texture, GLint level) {
    glFramebufferTexture2D(target, attachment, GL_TEXTURE_2D_MULTISAMPLE, texture, level);
    }
  //}}}
  //{{{
  void renderbuffer_storage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    glRenderbufferStorageMultisample(target, 4, internalformat, width, height);
    }
  //}}}

  //{{{
  void allocate_depth_texture(GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT32F, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
  //}}}
  //{{{
  void allocate_rgba_texture(GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
  //}}}

  //{{{
  void resize_rgba_texture(GLuint texture, GLsizei width, GLsizei height) {

    glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, texture);

    GLint internal_format;
    glGetTexLevelParameteriv (GL_TEXTURE_2D_MULTISAMPLE, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    glTexImage2DMultisample (GL_TEXTURE_2D_MULTISAMPLE, 4, internal_format, width, height, GL_TRUE);
    glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, 0);
    }
  //}}}
  //{{{
  void resize_depth_texture(GLuint texture, GLsizei width, GLsizei height) {
    resize_rgba_texture (texture, width, height);
    }
  //}}}
  };
//}}}

//{{{
Framebuffer::Framebuffer()
    : m_fbo(0), m_color(0), m_normal(0), m_depth(0), m_pimpl (new Default()) {

  // Create framebuffer object.
  glGenFramebuffers (1, &m_fbo);

  // Initialize.
  bind();
  initialize();
  unbind();
  }
//}}}
//{{{
Framebuffer::~Framebuffer() {

  bind();
  remove_and_delete_attachments();
  unbind();

  glDeleteFramebuffers(1, &m_fbo);
  }
//}}}

GLuint Framebuffer::color_texture() { return m_color; }
//{{{
void Framebuffer::enable_depth_texture() {

  bind();

  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
  glDeleteRenderbuffers (1, &m_depth);

  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);

  glGenTextures (1, &m_depth);
  m_pimpl->allocate_depth_texture (m_depth, viewport[2], viewport[3]);
  m_pimpl->framebuffer_texture_2d (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depth, 0);

  unbind();
  }
//}}}
//{{{
void Framebuffer::disable_depth_texture() {

  bind();

  m_pimpl->framebuffer_texture_2d (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 0, 0);
  glDeleteTextures (1, &m_depth);

  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);

  glGenRenderbuffers (1, &m_depth);
  glBindRenderbuffer (GL_RENDERBUFFER, m_depth);
  m_pimpl->renderbuffer_storage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, viewport[2], viewport[3]);
  glBindRenderbuffer (GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

  unbind();
  }
//}}}
GLuint Framebuffer::depth_texture() { return m_depth; }

//{{{
void Framebuffer::attach_normal_texture() {

  bind();

  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);

  glGenTextures (1, &m_normal);
  m_pimpl->allocate_rgba_texture (m_normal, viewport[2], viewport[3]);
  m_pimpl->framebuffer_texture_2d (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_normal, 0);

  GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers (2, buffers);

  unbind();
  }
//}}}
//{{{
void Framebuffer::detach_normal_texture() {

  bind();

  m_pimpl->framebuffer_texture_2d (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
  glDeleteTextures (1, &m_normal);

  GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers (1, buffers);

  unbind();
  }
//}}}
GLuint Framebuffer::normal_texture() { return m_normal; }

//{{{
void Framebuffer::set_multisample (bool enable) {

  if (m_pimpl->multisample() != enable) {
    bind();

    GLint type;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER,
                                           GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
    remove_and_delete_attachments();

    if (m_pimpl->multisample())
      m_pimpl = unique_ptr<Framebuffer::Impl>(new Framebuffer::Default());
    else
      m_pimpl = unique_ptr<Framebuffer::Impl>(new Framebuffer::Multisample());

    initialize();
    if (type == GL_TEXTURE) {
      attach_normal_texture();
      enable_depth_texture();
      }

    #ifndef NDEBUG
      GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE)
        cLog::log (LOGERROR, fmt::format ("{}", GLviz::getGlFramebufferStatusString (status)));

      GLenum gl_error = glGetError();
      if (GL_NO_ERROR != gl_error)
        cLog::log (LOGERROR, fmt::format ("{}", GLviz::getGlErrorString (gl_error)));
    #endif

    unbind();
    }
  }
//}}}

void Framebuffer::bind() { glBindFramebuffer (GL_FRAMEBUFFER, m_fbo); }
void Framebuffer::unbind() { glBindFramebuffer (GL_FRAMEBUFFER, 0); }

//{{{
void Framebuffer::resize (GLint width, GLint height) {

  bind();

  GLenum attachment[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

  for (unsigned int i(0); i < 2; ++i) {
    GLint type;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER, attachment[i],
                                           GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);

    GLint name;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER, attachment[i],
                                           GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
    if (type == GL_TEXTURE)
      m_pimpl->resize_rgba_texture (name, width, height);
    }

    {
    GLint type;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER,
                                           GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
    GLint name;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER,
                                           GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
    switch (type) {
      case GL_TEXTURE:
        m_pimpl->resize_depth_texture(name, width, height);
        break;

      case GL_RENDERBUFFER:
        glBindRenderbuffer (GL_RENDERBUFFER, name);
        m_pimpl->renderbuffer_storage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glBindRenderbuffer (GL_RENDERBUFFER, 0);
        break;

      case GL_NONE:
      default:
        break;
      }
    }

  #ifndef NDEBUG
    GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
      cLog::log (LOGERROR, fmt::format ("{}", GLviz::getGlFramebufferStatusString (status)));

    GLenum gl_error = glGetError();
    if (GL_NO_ERROR != gl_error)
      cLog::log (LOGERROR, fmt::format ("{}", GLviz::getGlErrorString (gl_error)));
  #endif

  unbind();
  }
//}}}

// private
//{{{
void Framebuffer::initialize() {

  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);

  // Attach color texture to framebuffer object.
  glGenTextures (1, &m_color);
  m_pimpl->allocate_rgba_texture (m_color, viewport[2], viewport[3]);
  m_pimpl->framebuffer_texture_2d (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_color, 0);

  // Attach renderbuffer object to framebuffer object.
  glGenRenderbuffers (1, &m_depth);
  glBindRenderbuffer (GL_RENDERBUFFER, m_depth);
  m_pimpl->renderbuffer_storage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, viewport[2], viewport[3]);
  glBindRenderbuffer (GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

  GLenum buffers[] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers (1, buffers);

  #ifndef NDEBUG
    GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
      cLog::log (LOGERROR, fmt::format ("{}", GLviz::getGlFramebufferStatusString (status)));
  #endif
  }
//}}}
//{{{
void Framebuffer::remove_and_delete_attachments() {

  GLenum attachment[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_DEPTH_ATTACHMENT };

  for (unsigned int i(0); i < 3; ++i) {
    GLint type;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER,
                                           attachment[i], GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);

    GLint name;
    glGetFramebufferAttachmentParameteriv (GL_FRAMEBUFFER,
                                           attachment[i], GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);

    switch (type) {
      case GL_TEXTURE:
        m_pimpl->framebuffer_texture_2d (GL_FRAMEBUFFER, attachment[i], 0, 0);
        glDeleteTextures (1, reinterpret_cast<const GLuint*>(&name));
        break;

      case GL_RENDERBUFFER:
        glFramebufferRenderbuffer (GL_FRAMEBUFFER, attachment[i], GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers (1, reinterpret_cast<const GLuint*>(&name));
        break;

      case GL_NONE:
      default:
        break;
      }
    }
  }
//}}}
//}}}
//{{{  ProgramAttribute
//{{{
ProgramAttribute::ProgramAttribute()
    : m_ewa_filter(false), m_backface_culling(false),
      m_visibility_pass(true), m_smooth(false), m_color_material(false),
      m_pointsize_method(0) {

  initShader();
  initProgram();
  }
//}}}
//{{{
void ProgramAttribute::set_smooth (bool enable) {

  if (m_smooth != enable) {
    m_smooth = enable;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_ewa_filter (bool enable) {

  if (m_ewa_filter != enable) {
    m_ewa_filter = enable;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_backface_culling (bool enable) {

  if (m_backface_culling != enable) {
    m_backface_culling = enable;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_visibility_pass (bool enable) {

  if (m_visibility_pass != enable) {
    m_visibility_pass = enable;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_color_material (bool enable) {

  if (m_color_material != enable) {
    m_color_material = enable;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_pointsize_method (unsigned int pointsize_method) {

  if (m_pointsize_method != pointsize_method) {
    m_pointsize_method = pointsize_method;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramAttribute::initShader() {

  mAttributeVs.load (kAttributeVsGlsl);
  mAttributeFs.load (kAttributeFsGlsl);
  mLightingVs.load (kLightingGlsl);
  }
//}}}
//{{{
void ProgramAttribute::initProgram() {

  try {
    detach_all();

    attach_shader (mAttributeVs);
    attach_shader (mAttributeFs);
    attach_shader (mLightingVs);

    // edit shader defines
    map <string, int> defines;
    defines.insert (make_pair ("EWA_FILTER", m_ewa_filter ? 1 : 0));
    defines.insert (make_pair ("BACKFACE_CULLING", m_backface_culling ? 1 : 0));
    defines.insert (make_pair ("VISIBILITY_PASS", m_visibility_pass ? 1 : 0));
    defines.insert (make_pair ("SMOOTH", m_smooth ? 1 : 0));
    defines.insert (make_pair ("COLOR_MATERIAL", m_color_material ? 1 : 0));
    defines.insert (make_pair ("POINTSIZE_METHOD", static_cast<int>(m_pointsize_method)));

    mAttributeVs.compile (defines);
    mAttributeFs.compile (defines);
    mLightingVs.compile (defines);
    }
  catch (shader_compilation_error const& e) {
    cLog::log(LOGERROR, fmt::format ("ProgramAttribute::initProgram - failed compile {}", e.what()));
    exit (EXIT_FAILURE);
    }

  try {
    link();
    }
  catch (shader_link_error const& e) {
    cLog::log (LOGERROR, fmt::format ("ProgramAttribute::initProgram - failed link {}", e.what()));
    exit (EXIT_FAILURE);
    }

  try {
    setUniformBlockBind ("Camera", 0);
    setUniformBlockBind ("Raycast", 1);
    setUniformBlockBind ("Frustum", 2);
    setUniformBlockBind ("Parameter", 3);
    }
  catch (uniform_not_found_error const& e) {
    cLog::log (LOGERROR, fmt::format ("ProgramAttribute::initProgram - failed setUniformBlockBind {}", e.what()));
    }
  }
//}}}
//}}}
//{{{  ProgramFinal
//{{{
ProgramFinal::ProgramFinal()
    : m_smooth(false), m_multisampling(false) {

  initShader();
  initProgram();
  }
//}}}
//{{{
void ProgramFinal::set_smooth (bool enable) {

  if (m_smooth != enable) {
    m_smooth = enable;
    initProgram();
    }
  }
//}}}
//{{{
void ProgramFinal::set_multisampling (bool enable) {

  if (m_multisampling != enable) {
    m_multisampling = enable;
    initProgram();
    }
  }

//}}}
//{{{
void ProgramFinal::initShader() {

  m_Final_vs_obj.load (kFinalVsGlsl);
  m_Final_fs_obj.load (kFinalFsGlsl);
  m_lighting_fs_obj.load (kLightingGlsl);

  attach_shader (m_Final_vs_obj);
  attach_shader (m_Final_fs_obj);
  attach_shader (m_lighting_fs_obj);
  }
//}}}
//{{{
void ProgramFinal::initProgram() {

  try {
    // edit shader defines
    map <string, int> defines;
    defines.insert (make_pair ("SMOOTH", m_smooth ? 1 : 0));
    defines.insert (make_pair ("MULTISAMPLING", m_multisampling ? 1 : 0));

    m_Final_vs_obj.compile (defines);
    m_Final_fs_obj.compile (defines);
    m_lighting_fs_obj.compile (defines);
    }
  catch (shader_compilation_error const& e) {
    cLog::log (LOGERROR, fmt::format ("ProgramFinal::initProgram - failed compile {}", e.what()));
    exit (EXIT_FAILURE);
    }

  try {
    link();
    }
  catch (shader_link_error const& e) {
    cLog::log (LOGERROR, fmt::format ("ProgramFinal::initProgram - failed link {}", e.what()));
    exit (EXIT_FAILURE);
    }

  try {
    setUniformBlockBind ("Camera", 0);
    setUniformBlockBind ("Raycast", 1);
    setUniformBlockBind ("Parameter", 3);
    }
  catch (uniform_not_found_error const& e) {
    cLog::log (LOGERROR, fmt::format ("ProgramFinal::initProgram - failed setUniformBlockBindto {}", e.what()));
    }
  }
//}}}
//}}}

// cSplatRender
//{{{
cSplatRender::cSplatRender (GLviz::Camera const& camera)
    : cRender(camera),
      m_soft_zbuffer(true),
      m_smooth(false),
      m_ewa_filter(false),
      m_pointsize_method(0),
      m_backface_culling(false),
      m_epsilon(1.0f * 1e-3f),
      m_radius_scale(1.0f),
      m_ewa_radius(1.0f) {

  m_uniform_camera.bindBufferBase (0);
  m_uniform_raycast.bindBufferBase (1);
  m_uniform_frustum.bindBufferBase (2);
  m_uniform_parameter.bindBufferBase (3);

  setupProgramObjects();
  setup_filter_kernel();
  setup_screen_size_quad();
  setup_vertex_array_buffer_object();
  }
//}}}
//{{{
cSplatRender::~cSplatRender() {

  glDeleteVertexArrays (1, &m_vao);
  glDeleteBuffers (1, &m_vbo);

  glDeleteBuffers (1, &m_rect_vertices_vbo);
  glDeleteBuffers (1, &m_rect_texture_uv_vbo);
  glDeleteVertexArrays (1, &m_rect_vao);

  glDeleteTextures (1, &m_filter_kernel);
  }
//}}}

bool cSplatRender::smooth() const { return m_smooth; }
//{{{
void cSplatRender::set_smooth (bool enable) {

  if (m_smooth != enable) {
    m_smooth = enable;

    m_attribute.set_smooth(enable);
    m_Final.set_smooth(enable);

    if (m_smooth) {
      m_fbo.enable_depth_texture();
      m_fbo.attach_normal_texture();
      }
    else {
      m_fbo.disable_depth_texture();
      m_fbo.detach_normal_texture();
      }
    }
  }
//}}}

//{{{  soft z
bool cSplatRender::soft_zbuffer() const { return m_soft_zbuffer; }
//{{{
void cSplatRender::set_soft_zbuffer (bool enable) {

  if (m_soft_zbuffer != enable) {
    if (!enable) {
       m_ewa_filter = false;
       m_attribute.set_ewa_filter(false);
       }

    m_soft_zbuffer = enable;
    }
  }
//}}}

float cSplatRender::getSoftZbufferEpsilon() const { return m_epsilon; }
void cSplatRender::setSoftZbufferEpsilon (float epsilon) { m_epsilon = epsilon; }
//}}}
//{{{  EWA
bool cSplatRender::ewa_filter() const { return m_ewa_filter; }
//{{{
void cSplatRender::set_ewa_filter (bool enable) {

  if (m_soft_zbuffer && m_ewa_filter != enable) {
    m_ewa_filter = enable;
    m_attribute.set_ewa_filter(enable);
    }
  }
//}}}

float cSplatRender::ewa_radius() const { return m_ewa_radius; }
void cSplatRender::set_ewa_radius (float ewa_radius) { m_ewa_radius = ewa_radius; }
//}}}

unsigned int cSplatRender::pointsize_method() const { return m_pointsize_method; }
//{{{
void cSplatRender::set_pointsize_method (unsigned int pointsize_method) {

  if (m_pointsize_method != pointsize_method) {
    m_pointsize_method = pointsize_method;
    m_visibility.set_pointsize_method(pointsize_method);
    m_attribute.set_pointsize_method(pointsize_method);
    }
  }
//}}}

float cSplatRender::radius_scale() const { return m_radius_scale; }
void cSplatRender::set_radius_scale (float radius_scale) { m_radius_scale = radius_scale; }

// overrides
//{{{
void cSplatRender::setMultiSample (bool enable) {

  if (getMultiSample() != enable) {
    cRender::setMultiSample (enable);
    m_Final.set_multisampling (enable);
    m_fbo.set_multisample (enable);
    }
  }
//}}}
//{{{
void cSplatRender::setBackFaceCull (bool enable) {

  if (getBackFaceCull() != enable) {
    cRender::setMultiSample (enable);
    m_visibility.set_backface_culling (enable);
    m_attribute.set_backface_culling (enable);
    }
  }
//}}}

void cSplatRender::resize (int width, int height) { m_fbo.resize (width, height); }
//{{{
void cSplatRender::render (cModel* model) {

  cSurfelModel* surfelModel = dynamic_cast<cSurfelModel*>(model);

  m_fbo.bind();

  glDepthMask (GL_TRUE);
  glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glClearDepth (1.0);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mNumSurfels = surfelModel->getSize();
  if (mNumSurfels > 0) {
    glBindBuffer (GL_ARRAY_BUFFER, m_vbo);
    glBufferData (GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    glBufferData (GL_ARRAY_BUFFER, sizeof(cSurfelModel::cSurfel) * mNumSurfels,
                                   (void*)(surfelModel->getArray()), GL_DYNAMIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);

    if (getMultiSample()) {
      glEnable (GL_MULTISAMPLE);
      glEnable (GL_SAMPLE_SHADING);
      glMinSampleShading (4.0);
      }

    if (m_soft_zbuffer)
      renderPass (true);

    renderPass (false);

    if (getMultiSample()) {
      glDisable (GL_MULTISAMPLE);
      glDisable (GL_SAMPLE_SHADING);
      }
    }

  m_fbo.unbind();

  //{{{  finalise
  if (getMultiSample()) {
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, m_fbo.color_texture());

    if (m_smooth) {
      glActiveTexture (GL_TEXTURE1);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, m_fbo.normal_texture());
      glActiveTexture (GL_TEXTURE2);
      glBindTexture (GL_TEXTURE_2D_MULTISAMPLE, m_fbo.depth_texture());
      }
    }
  else {
    glActiveTexture (GL_TEXTURE0);
    glBindTexture (GL_TEXTURE_2D, m_fbo.color_texture());

    if (m_smooth) {
      glActiveTexture (GL_TEXTURE1);
      glBindTexture (GL_TEXTURE_2D, m_fbo.normal_texture());
      glActiveTexture (GL_TEXTURE2);
      glBindTexture (GL_TEXTURE_2D, m_fbo.depth_texture());
      }
    }

  m_Final.use();

  try {
    setupUniforms (m_Final);
    m_Final.setUniform1i ("color_texture", 0);
    if (m_smooth) {
      m_Final.setUniform1i ("normal_texture", 1);
      m_Final.setUniform1i ("depth_texture", 2);
      }
    }
  catch (uniform_not_found_error const& e) {
    cLog::log (LOGERROR, fmt::format ("failed to set a uniform variable {}", e.what()));
    }

  glBindVertexArray (m_rect_vao);
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray (0);
  //}}}

  #ifndef NDEBUG
    GLenum gl_error = glGetError();
    if (GL_NO_ERROR != gl_error)
      cLog::log (LOGERROR, fmt::format ("{}", GLviz::getGlErrorString (gl_error)));
  #endif
  }
//}}}
//{{{
bool cSplatRender::keyboard (SDL_Keycode key) {

  switch (key) {
    case SDLK_5: set_smooth (!smooth()); return true;
    case SDLK_c: setColorMaterial (!getColorMaterial()); return true;
    case SDLK_z: set_soft_zbuffer (!soft_zbuffer()); return true;
    case SDLK_u: set_ewa_filter (!ewa_filter()); return true;
    case SDLK_t: set_pointsize_method ((pointsize_method() + 1) % 4); return true;
    }

  return false;
  }
//}}}
//{{{
void cSplatRender::gui() {

  ImGui::SetNextItemOpen (true, ImGuiCond_Once);
  if (ImGui::CollapsingHeader ("Surface Splatting")) {
    int shadingMethod = smooth() ? 1 : 0;
    if (ImGui::Combo ("Shading", &shadingMethod, "Flat\0Smooth\0\0"))
      set_smooth (shadingMethod > 0 ? true : false);

    //{{{  material
    ImGui::Separator();
    int color_material = getColorMaterial() ? 1 : 0;
    if (ImGui::Combo ("Color", &color_material, "Surfel\0Material\0\0"))
      setColorMaterial (color_material > 0 ? true : false);

    float materialColor[3] =  { getMaterialColor()[0],
                                getMaterialColor()[1],
                                getMaterialColor()[2] };
    if (ImGui::ColorEdit3 ("Material color", materialColor))
      setMaterialColor (Eigen::Vector3f(materialColor[0], materialColor[1], materialColor[2]));

    float materialShininess = getMaterialShininess();
    if (ImGui::DragFloat ("Material shininess", &materialShininess, 0.05f, 1e-12f, 1000.0f))
      setMaterialShininess (min(max( 1e-12f, materialShininess), 1000.0f));
    //}}}
    //{{{  soft z
    ImGui::Separator();
    bool softZbuffer = soft_zbuffer();
    if (ImGui::Checkbox("Soft z-buffer", &softZbuffer))
      set_soft_zbuffer (softZbuffer);

    float soft_zbuffer_epsilon = getSoftZbufferEpsilon();
    if (ImGui::DragFloat ("Soft z-buffer epsilon", &soft_zbuffer_epsilon, 1e-5f, 1e-5f, 1.0f, "%.5f"))
      setSoftZbufferEpsilon (min(max(1e-5f, soft_zbuffer_epsilon), 1.0f));
    //}}}
    //{{{  ewa
    ImGui::Separator();
    bool ewaFilter = ewa_filter();
    if (ImGui::Checkbox ("EWA filter", &ewaFilter))
      set_ewa_filter (ewaFilter);

    float ewaRadius = ewa_radius();
    if (ImGui::DragFloat ("EWA radius", &ewaRadius, 1e-3f, 0.1f, 4.0f))
      set_ewa_radius (ewaRadius);
    //}}}

    ImGui::Separator();
    int pointSize = pointsize_method();
    if (ImGui::Combo ("Point size", &pointSize, "PBP\0BHZK05\0WHA+07\0ZRB+04\0\0"))
      set_pointsize_method (pointSize);

    float radiusScale = radius_scale();
    if (ImGui::DragFloat ("Radius scale", &radiusScale, 0.001f, 1e-6f, 2.0f))
      set_radius_scale (min(max( 1e-6f, radiusScale), 2.0f));

    ImGui::Separator();
    bool multiSample = getMultiSample();
    if (ImGui::Checkbox ("MultiSample x4", &multiSample))
      setMultiSample (multiSample);

    bool backFaceCull = getBackFaceCull();
    if (ImGui::Checkbox ("Backface cull", &backFaceCull))
      setBackFaceCull (backFaceCull);
    }
  }
//}}}

// private
//{{{
void cSplatRender::setupProgramObjects() {

  m_visibility.set_visibility_pass();
  m_visibility.set_pointsize_method (m_pointsize_method);
  m_visibility.set_backface_culling (m_backface_culling);

  m_attribute.set_visibility_pass (false);
  m_attribute.set_pointsize_method (m_pointsize_method);
  m_attribute.set_backface_culling (m_backface_culling);
  m_attribute.set_color_material (mColorMaterial);
  m_attribute.set_ewa_filter (m_ewa_filter);
  m_attribute.set_smooth (m_smooth);

  m_Final.set_multisampling (getMultiSample());
  m_Final.set_smooth (m_smooth);
  }
//}}}
//{{{
inline void cSplatRender::setup_filter_kernel() {

  const float sigma2 = 0.316228f; // Sqrt(0.1).

  GLfloat yi[256];
  for (unsigned int i = 0; i < 256; ++i) {
    float x = static_cast<GLfloat>(i) / 255.0f;
    float const w = x * x / (2.0f * sigma2);
    yi[i] = std::exp(-w);
    }

  glGenTextures (1, &m_filter_kernel);
  glBindTexture (GL_TEXTURE_1D, m_filter_kernel);
  glPixelStorei (GL_UNPACK_ALIGNMENT, 4);
  glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage1D (GL_TEXTURE_1D, 0, GL_R32F, 256, 0, GL_RED, GL_FLOAT, yi);
  }
//}}}
//{{{
inline void cSplatRender::setup_screen_size_quad() {

  float rect_vertices[12] = { 1.0f,  1.0f, 0.0f,
                              1.0f, -1.0f, 0.0f,
                             -1.0f,  1.0f, 0.0f,
                             -1.0f, -1.0f, 0.0f };

  float rect_texture_uv[8] = { 1.0f, 1.0f,
                               1.0f, 0.0f,
                               0.0f, 1.0f,
                               0.0f, 0.0f };

  glGenBuffers (1, &m_rect_vertices_vbo);
  glBindBuffer (GL_ARRAY_BUFFER, m_rect_vertices_vbo);
  glBufferData (GL_ARRAY_BUFFER, 12 * sizeof(float), rect_vertices, GL_STATIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  glGenBuffers (1, &m_rect_texture_uv_vbo);
  glBindBuffer (GL_ARRAY_BUFFER, m_rect_texture_uv_vbo);
  glBufferData (GL_ARRAY_BUFFER, 8 * sizeof(float), rect_texture_uv, GL_STATIC_DRAW);
  glBindBuffer (GL_ARRAY_BUFFER, 0);

  glGenVertexArrays (1, &m_rect_vao);
  glBindVertexArray (m_rect_vao);

  glBindBuffer (GL_ARRAY_BUFFER, m_rect_vertices_vbo);
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<const GLvoid*>(0));

  glBindBuffer (GL_ARRAY_BUFFER, m_rect_texture_uv_vbo);
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<const GLvoid*>(0));

  glBindVertexArray (0);
  }
//}}}
//{{{
void cSplatRender::setup_vertex_array_buffer_object() {

  glGenBuffers (1, &m_vbo);

  glGenVertexArrays (1, &m_vao);
  glBindVertexArray (m_vao);

  glBindBuffer (GL_ARRAY_BUFFER, m_vbo);

  // Center c.
  glEnableVertexAttribArray (0);
  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(cSurfelModel::cSurfel), reinterpret_cast<const GLfloat*>(0));

  // Tagent vector u.
  glEnableVertexAttribArray (1);
  glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, sizeof(cSurfelModel::cSurfel), reinterpret_cast<const GLfloat*>(12));

  // Tangent vector v.
  glEnableVertexAttribArray (2);
  glVertexAttribPointer (2, 3, GL_FLOAT, GL_FALSE, sizeof(cSurfelModel::cSurfel), reinterpret_cast<const GLfloat*>(24));

  // Clipping plane p.
  glEnableVertexAttribArray (3);
  glVertexAttribPointer (3, 3, GL_FLOAT, GL_FALSE, sizeof(cSurfelModel::cSurfel), reinterpret_cast<const GLfloat*>(36));

  // Color rgba.
  glEnableVertexAttribArray (4);
  glVertexAttribPointer (4, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(cSurfelModel::cSurfel), reinterpret_cast<const GLbyte*>(48));

  glBindVertexArray (0);
  }
//}}}
//{{{
void cSplatRender::setupUniforms (glProgram& program) {

  m_uniform_camera.set_buffer_data (mCamera);

  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);
  GLviz::Frustum view_frustum = mCamera.get_frustum();

  m_uniform_raycast.set_buffer_data (mCamera.get_projection_matrix().inverse(), viewport);

  Eigen::Vector4f frustum_plane[6];

  Eigen::Matrix4f const& projection_matrix = mCamera.get_projection_matrix();
  for (unsigned int i(0); i < 6; ++i)
    frustum_plane[i] = projection_matrix.row(3) + (-1.0f + 2.0f * static_cast<float>(i % 2)) *
                       projection_matrix.row(i / 2);

  for (unsigned int i(0); i < 6; ++i)
    frustum_plane[i] = (1.0f / frustum_plane[i].block<3, 1>( 0, 0).norm()) * frustum_plane[i];
  m_uniform_frustum.set_buffer_data (frustum_plane);

  m_uniform_parameter.set_buffer_data (getMaterialColor(), getMaterialShininess(),
                                       m_radius_scale, m_ewa_radius, m_epsilon);
  }
//}}}

//{{{
void cSplatRender::renderPass (bool depth_only) {

  glEnable (GL_DEPTH_TEST);
  glEnable (GL_PROGRAM_POINT_SIZE);

  if (!depth_only && m_soft_zbuffer) {
    glEnable (GL_BLEND);
    glBlendEquationSeparate (GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate (GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE);
    }

  glProgram& program = depth_only ? m_visibility : m_attribute;
  program.use();

  if (depth_only) {
    glDepthMask (GL_TRUE);
    glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
  else {
    if (m_soft_zbuffer)
      glDepthMask (GL_FALSE);
    else
      glDepthMask (GL_TRUE);

    glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

  setupUniforms (program);

  if (!depth_only && m_soft_zbuffer && m_ewa_filter) {
    glActiveTexture (GL_TEXTURE1);
    glBindTexture (GL_TEXTURE_1D, m_filter_kernel);
    program.setUniform1i ("filter_kernel", 1);
    }

  glBindVertexArray (m_vao);
  glDrawArrays (GL_POINTS, 0, (GLsizei)mNumSurfels);
  glBindVertexArray (0);

  program.unuse();

  glDisable (GL_PROGRAM_POINT_SIZE);
  glDisable (GL_BLEND);
  glDisable (GL_DEPTH_TEST);
  }
//}}}
