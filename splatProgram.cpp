//{{{  includes
#include <iostream>
#include <cstddef>

#include "splatProgram.h"

#include "../common/date.h"
#include "../common/cLog.h"

using namespace std;
//}}}

namespace {
  // shaders
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

// ProgramAttribute
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

// ProgramFinal
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
