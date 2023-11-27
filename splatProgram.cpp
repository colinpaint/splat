//{{{  includes
#include "splatProgram.h"

#include <iostream>
#include <cstddef>

using namespace std;
//}}}

namespace {
  //{{{
  const std::string kAttributeVsGlsl =
    "#version 330\n"

    "#define VISIBILITY_PASS    0\n"
    "#define BACKFACE_CULLING   0\n"
    "#define SMOOTH             0\n"
    "#define COLOR_MATERIAL     0\n"
    "#define EWA_FILTER         0\n"
    "#define POINTSIZE_METHOD   0\n"

    "layout(std140, column_major) uniform Camera {\n"
      "mat4 modelview_matrix;\n"
      "mat4 modelview_matrix_it;\n"
      "mat4 projection_matrix;\n"
      "};\n"

    "layout(std140, column_major) uniform Raycast {\n"
      "mat4 projection_matrix_inv;\n"
      "vec4 viewport;\n"
      "};\n"

    "layout(std140) uniform Frustum {\n"
      "vec4 frustum_plane[6];\n"
      "};\n"

    "layout(std140) uniform Parameter {\n"
      "vec3 material_color;\n"
      "float material_shininess;\n"
      "float radius_scale;\n"
      "float ewa_radius;\n"
      "float epsilon;\n"
      "};\n"

    "#define ATTR_CENTER 0\n"
    "layout(location = ATTR_CENTER) in vec3 c;\n"

    "#define ATTR_T1 1\n"
    "layout(location = ATTR_T1) in vec3 u;\n"

    "#define ATTR_T2 2\n"
    "layout(location = ATTR_T2) in vec3 v;\n"

    "#define ATTR_PLANE 3\n"
    "layout(location = ATTR_PLANE) in vec3 p;\n"

    "#define ATTR_COLOR 4\n"
    "layout(location = ATTR_COLOR) in vec4 rgba;\n"

    "out block {\n"
      "flat out vec3 c_eye;\n"
      "flat out vec3 u_eye;\n"
      "flat out vec3 v_eye;\n"
      "flat out vec3 p;\n"
      "flat out vec3 n_eye;\n"

      "#if !VISIBILITY_PASS\n"
        "#if EWA_FILTER\n"
          "flat out vec2 c_scr;\n"
        "#endif\n"
        "flat out vec3 color;\n"
      "#endif\n"
      "}\n"
    "Out;\n"

    "#if !VISIBILITY_PASS\n"
      "vec3 lighting(vec3 n_eye, vec3 v_eye, vec3 color, float shininess);\n"
    "#endif\n"

    "void intersect(in vec4 v1, in vec4 v2, in int p, out int n_pts, out vec4[2] pts) {\n"
      "int i = p / 2;\n"
      "float j = float(-1 + 2 * (p % 2));\n"

      "float b1 = v1.w + float(j) * v1[i];\n"
      "float b2 = v2.w + float(j) * v2[i];\n"

      "bool tb1 = b1 > 0.0;\n"
      "bool tb2 = b2 > 0.0;\n"

      "n_pts = 0;\n"

      "if (tb1 && tb2) {\n"
        "pts[0] = v2;\n"
        "n_pts = 1;\n"
        "}\n"
      "else if (tb1 && !tb2) {\n"
        "float a = b1 / (b1 - b2);\n"
        "pts[0] = (1.0 - a) * v1 + a * v2;\n"
        "n_pts = 1;\n"
        "}\n"
      "else if (!tb1 && tb2) {\n"
        "float a = b1 / (b1 - b2);\n"
        "pts[0] = (1.0 - a) * v1 + a * v2;\n"
        "pts[1] = v2;\n"
        "n_pts = 2;\n"
        "}\n"
      "}\n"

    "void clip_polygon(in vec4 p0[4], out int n_pts, out vec4 p1[8]) {\n"
      "vec4 p[8];\n"
      "int n = 4;\n"

      "p[0] = p0[0];\n"
      "p[1] = p0[1];\n"
      "p[2] = p0[2];\n"
      "p[3] = p0[3];\n"

      "for (int i = 0; i < 6; ++i) {\n"
        "int k = 0;\n"

        "for (int j = 0; j < n; ++j) {\n"
          "int n_pts;\n"
          "vec4 pts[2];\n"
          "intersect(p[j], p[(j + 1) % n], i, n_pts, pts);\n"

          "if (n_pts == 1) {\n"
            "p1[k++] = pts[0];\n"
            "}\n"
          "else if (n_pts == 2) {\n"
            "p1[k++] = pts[0];\n"
            "p1[k++] = pts[1];\n"
            "}\n"
          "}\n"

        "for (int j = 0; j < k; ++j) {\n"
         "p[j] = p1[j];\n"
          "}\n"

        "n = k;\n"
        "}\n"

      "n_pts = n;\n"
      "}\n"

    "void conic_Q(in vec3 u, in vec3 v, in vec3 c, out mat3 Q1) {\n"
    "             mat3 Q0 = mat3(vec3(1.0, 0.0, 0.0),\n"
    "             vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, -1.0));\n"

      "mat3 Sinv = transpose(mat3(cross(v, c), cross(u, c), cross(u, v)));\n"
      "Sinv[0][1] = -Sinv[0][1];\n"
      "Sinv[1][1] = -Sinv[1][1];\n"
      "Sinv[2][1] = -Sinv[2][1];\n"

      "mat3 Pinv = mat3 (vec3(projection_matrix_inv[0]),\n"
                         "vec3(projection_matrix_inv[1]),\n"
                         "vec3(projection_matrix_inv[2]));\n"
      "Pinv[2][2] = -1.0;\n"

      "mat3 Minv = Sinv * Pinv;\n"
      "Q1 = transpose(Minv) * Q0 * Minv;\n"
      "}\n"

    "void pointsprite(in vec3 c, in vec3 u, in vec3 v, out vec4 p_scr, out vec2 w) {\n"
      "#if POINTSIZE_METHOD == 0\n"
        // This method obtains the position and bounds of a splat by
        // clipping and perspectively projecting a bounding polygon.
        "mat4 M = projection_matrix * mat4(\n"
            "vec4(u, 0.0), vec4(v, 0.0), vec4(c, 0.0),\n"
            "vec4(0.0, 0.0, 0.0, 1.0));\n"

        "vec4 p0[4];\n"
        "p0[0] = M * vec4(1.0);\n"
        "p0[1] = M * vec4(1.0, -1.0, 1.0, 1.0);\n"
        "p0[2] = M * vec4(-1.0, -1.0, 1.0, 1.0);\n"
        "p0[3] = M * vec4(-1.0, 1.0, 1.0, 1.0);\n"

        "int n_pts;\n"
        "vec4 p1[8];\n"

        "clip_polygon(p0, n_pts, p1);\n"

        "if (n_pts == 0) {\n"
          "p_scr = vec4(1.0, 0.0, 0.0, 0.0);\n"
          "w = vec2(0.0);\n"
          "}\n"
        "else {\n"
          "vec2 p_min = vec2(1.0);\n"
          "vec2 p_max = vec2(-1.0);\n"

          "for (int i = 0; i < n_pts; ++i) {\n"
            "vec2 p1i = p1[i].xy / p1[i].w;\n"
            "p_min = min(p_min, p1i);\n"
            "p_max = max(p_max, p1i);\n"
            "}\n"

          "w = 0.5 * (p_max - p_min);\n"

          "p_scr.xy = p_min + w;\n"
          "p_scr.z = 0.0;\n"
          "p_scr.w = 1.0;\n"
          "}\n"

      "#elif POINTSIZE_METHOD == 1\n"
        "p_scr = projection_matrix * vec4(c, 1.0);\n"
        "float p11 = projection_matrix[1][1];\n"
        "float r = max(length(u), length(v));\n"
        "w = vec2(0.0, r * p11 / abs(c.z));\n"
      "#elif POINTSIZE_METHOD == 2\n"
        // WHA+07.
        "float r = max(length(u), length(v));\n"
        "vec3 pl = vec3(\n"
        "    dot(frustum_plane[0], vec4(c, 1.0)),\n"
        "    dot(frustum_plane[2], vec4(c, 1.0)),\n"
        "    dot(frustum_plane[4], vec4(c, 1.0)));\n"
        "vec3 pr = vec3(\n"
        "    dot(frustum_plane[1], vec4(c, 1.0)),\n"
        "    dot(frustum_plane[3], vec4(c, 1.0)),\n"
        "    dot(frustum_plane[5], vec4(c, 1.0)));\n"

        "bool t_lr = (pl.x + r) > 0.0 && (pr.x + r) > 0.0;\n"
        "bool t_bt = (pl.y + r) > 0.0 && (pr.y + r) > 0.0;\n"
        "bool t_nf = (pl.z + r) > 0.0 && (pr.z + r) > 0.0;\n"

        "if (t_lr && t_bt && t_nf) {\n"
            "mat4x3 T = transpose(projection_matrix * mat3x4(vec4(u, 0.0), vec4(v, 0.0), vec4(c, 1.0) ));\n"

            "float d = dot(vec3(1.0, 1.0, -1.0), T[3] * T[3]);\n"
            "vec3 f = (1.0 / d) * vec3(1.0, 1.0, -1.0);\n"

            "vec3 p = vec3(dot(f, T[0] * T[3]), dot(f, T[1] * T[3]), dot(f, T[2] * T[3]));\n"

            "vec3 h0 = p * p - vec3(dot(f, T[0] * T[0]), dot(f, T[1] * T[1]), dot(f, T[2] * T[2]));\n"
            "vec3 h = sqrt(max(vec3(0.0), h0)) + vec3(0.0, 0.0, 1e-2);\n"

            "w = h.xy;\n"
            "p_scr = vec4(p.xy, 0.0, 1.0);\n"
        "}\n"
        "else {\n"
            "p_scr = vec4(1.0, 0.0, 0.0, 0.0);\n"
            "w = vec2(0.0);\n"
        "}\n"

      "#elif POINTSIZE_METHOD == 3\n"
        // ZRB+04.
        "mat3 Q;\n"
        "conic_Q(u, v, c, Q);\n"

        "float Qa = Q[0][0]; float Qb = Q[1][0];\n"
        "float Qc = Q[1][1]; float Qd = Q[2][0];\n"
        "float Qe = Q[2][1]; float Qf = -Q[2][2];\n"

        "float delta = Qa * Qc - Qb * Qb;\n"

        "if (delta > 0.0) {\n"
            "vec2 p = (Qb * vec2(Qe, Qd) - vec2(Qc * Qd, Qa * Qe)) / delta;\n"
            "float bb = Qf - dot(vec2(Qd, Qe), p);\n"

            "mat3 Q2 = mat3(\n"
                "vec3(Qa, Qb, 0.0),\n"
                "vec3(Qb, Qc, 0.0),\n"
                "vec3(0.0, 0.0, -bb)) / bb;\n"

            "float delta2 = Q2[0][0] * Q2[1][1] - Q2[0][1] * Q2[0][1];\n"
            "vec2 h = sqrt(vec2(Q2[1][1], Q2[0][0]) / delta2);\n"

            // Put an upper bound on the point size since the
            // centralized conics method is numerically unstable.
            "w = clamp(h, 0.0, 0.1);\n"
            "p_scr = vec4(p, 0.0, 1.0);\n"
        "}\n"
        "else {\n"
            "p_scr = vec4(1.0, 0.0, 0.0, 0.0);\n"
            "w = vec2(0.0);\n"
        "}\n"
      "#endif\n"
      "}\n"

    "void main() {\n"
      "vec4 c_eye = modelview_matrix * vec4(c, 1.0);\n"
      "vec3 u_eye = radius_scale * mat3(modelview_matrix) * u;\n"
      "vec3 v_eye = radius_scale * mat3(modelview_matrix) * v;\n"
      "vec3 n_eye = normalize(cross(u_eye, v_eye));\n"

      "vec4 p_scr;\n"
      "vec2 w;\n"
      "pointsprite(c_eye.xyz, u_eye, v_eye, p_scr, w);\n"

      "#if !VISIBILITY_PASS\n"
        "#if SMOOTH\n"
          "#if COLOR_MATERIAL\n"
            "Out.color = material_color;\n"
          "#else\n"
            "Out.color = vec3(rgba);\n"
          "#endif\n"
        "#else\n"
          "#if COLOR_MATERIAL\n"
            "Out.color = lighting(n_eye, vec3(c_eye), material_color, material_shininess);\n"
          "#else\n"
            "Out.color = lighting(n_eye, vec3(c_eye), vec3(rgba), material_shininess);\n"
          "#endif\n"
        "#endif\n"
      "#endif\n"

      "#if BACKFACE_CULLING\n"
        // Backface culling
        "if (dot(n_eye, -vec3(c_eye)) > 0.0) {\n"
      "#endif\n"
          // Pointsprite position"
          "gl_Position = p_scr;\n"

          "Out.c_eye = vec3(c_eye);\n"
          "Out.u_eye = u_eye;\n"
          "Out.v_eye = v_eye;\n"
          "Out.p = p;\n"
          "Out.n_eye = n_eye;\n"

          // Pointsprite size. One additional pixel avoids artifacts.
          "float point_size = max(w[0] * viewport.z, w[1] * viewport.w) + 1.0;\n"

      "#if !VISIBILITY_PASS && EWA_FILTER\n"
        "Out.c_scr = vec2((p_scr.xy + 1.0) * viewport.zw * 0.5);\n"
        "gl_PointSize = max(2.0, point_size);\n"
      "#else\n"
        "gl_PointSize = point_size;\n"
      "#endif\n"

      "#if BACKFACE_CULLING\n"
        "}\n"
      "else {\n"
        "gl_Position = vec4(1.0, 0.0, 0.0, 0.0);\n"
        "}\n"
      "#endif\n"
      "}\n";
  //}}}
  //{{{
  const std::string kAttributeFsGlsl =
    "#version 330\n"

    "#define VISIBILITY_PASS  0\n"
    "#define SMOOTH           0\n"
    "#define EWA_FILTER       0\n"

    "layout(std140, column_major) uniform Camera {\n"
      "mat4 modelview_matrix;\n"
      "mat4 modelview_matrix_it;\n"
      "mat4 projection_matrix;\n"
      "};\n"

    "layout(std140, column_major) uniform Raycast {\n"
      "mat4 projection_matrix_inv;\n"
      "vec4 viewport;\n"
      "};\n"

    "layout(std140) uniform Parameter {\n"
      "vec3 material_color;\n"
      "float material_shininess;\n"
      "float radius_scale;\n"
      "float ewa_radius;\n"
      "float epsilon;\n"
      "};\n"

    "uniform sampler1D filter_kernel;\n"

    "in block {\n"
      "flat in vec3 c_eye;\n"
      "flat in vec3 u_eye;\n"
      "flat in vec3 v_eye;\n"
      "flat in vec3 p;\n"
      "flat in vec3 n_eye;\n"

      "#if !VISIBILITY_PASS\n"
        "#if EWA_FILTER\n"
          "flat in vec2 c_scr;\n"
        "#endif\n"
      "flat in vec3 color;\n"
      "#endif\n"
      "}\n"
    "In;\n"

    "#define FRAG_COLOR 0\n"
    "layout(location = FRAG_COLOR) out vec4 frag_color;\n"

    "#if !VISIBILITY_PASS\n"
      "#if SMOOTH\n"
        "#define FRAG_NORMAL 1\n"
          "layout(location = FRAG_NORMAL) out vec4 frag_normal;\n"
      "#endif\n"
    "#endif\n"

    "void main() {\n"
      "vec4 p_ndc = vec4(2.0 * (gl_FragCoord.xy - viewport.xy) / (viewport.zw) - 1.0, -1.0, 1.0);\n"
      "vec4 p_eye = projection_matrix_inv * p_ndc;\n"
      "vec3 qn = p_eye.xyz / p_eye.w;\n"
      "vec3 q = qn * dot(In.c_eye, In.n_eye) / dot(qn, In.n_eye);\n"
      "vec3 d = q - In.c_eye;\n"

      "vec2 u = vec2(dot(In.u_eye, d) / dot(In.u_eye, In.u_eye),\n"
                    "dot(In.v_eye, d) / dot(In.v_eye, In.v_eye));\n"

      "if (dot(vec3(u, 1.0), In.p) < 0) {\n"
        "discard;\n"
        "}\n"

      "float w3d = length(u);\n"
      "float zval = q.z;\n"

      "#if !VISIBILITY_PASS && EWA_FILTER\n"
        "float w2d = distance(gl_FragCoord.xy, In.c_scr) / ewa_radius;\n"
        "float dist = min(w2d, w3d);\n"

        // Avoid visual artifacts due to wrong z-values for fragments
        // being part of the low-pass filter, but outside of the
        // reconstruction filter.
        "if (w3d > 1.0) {\n"
          "zval = In.c_eye.z;\n"
          "}\n"
      "#else\n"
        "float dist = w3d;\n"
      "#endif\n"

      "if (dist > 1.0) {\n"
        "discard;\n"
        "}\n"

      "#if !VISIBILITY_PASS\n"
        "#if EWA_FILTER\n"
          "float alpha = texture(filter_kernel, dist).r;\n"
        "#else\n"
          "float alpha = 1.0;\n"
        "#endif\n"

        "frag_color = vec4(In.color, alpha);\n"

        "#if SMOOTH\n"
          "frag_normal = vec4(In.n_eye, alpha);\n"
        "#endif\n"
      "#endif\n"

      "#if VISIBILITY_PASS\n"
        "zval -= epsilon;\n"
      "#endif\n"

      "float depth = -projection_matrix[3][2] * (1.0 / zval) - projection_matrix[2][2];\n"

      "gl_FragDepth = (depth + 1.0) / 2.0;\n"
      "}\n";
  //}}}
  //{{{
  const std::string kFinalizationVsGlsl =
    "#version 330\n"

    "#define ATTR_POSITION 0\n"
    "layout(location = ATTR_POSITION) in vec3 position;\n"

    "#define ATTR_TEXTURE_UV 1\n"
    "layout(location = ATTR_TEXTURE_UV) in vec2 texture_uv;\n"

    "out block {\n"
    "  vec2 texture_uv;\n"
    "  }\n"
    "Out;\n"

    "void main() {\n"
    "  gl_Position = vec4(position, 1.0);\n"
    "  Out.texture_uv = texture_uv;\n"
    "  }\n";
  //}}}
  //{{{
  const std::string kFinalizationFsGlsl =
    "#version 330\n"

    "#define MULTISAMPLING  0\n"
    "#define SMOOTH         0\n"

    "layout(std140, column_major) uniform Camera {\n"
      "mat4 modelview_matrix;\n"
      "mat4 modelview_matrix_it;\n"
      "mat4 projection_matrix;\n"
      "};\n"

    "layout(std140, column_major) uniform Raycast {\n"
      "mat4 projection_matrix_inv;\n"
      "vec4 viewport;\n"
      "};\n"

    "layout(std140) uniform Parameter {\n"
      "vec3 material_color;\n"
      "float material_shininess;\n"
      "float radius_scale;\n"
      "float ewa_radius;\n"
      "float epsilon;\n"
      "};\n"

    "#if MULTISAMPLING\n"
      "uniform sampler2DMS color_texture;\n"
    "#else\n"
      "uniform sampler2D color_texture;\n"
    "#endif\n"

    "#if SMOOTH\n"
      "#if MULTISAMPLING\n"
        "uniform sampler2DMS normal_texture;\n"
        "uniform sampler2DMS depth_texture;\n"
      "#else\n"
        "uniform sampler2D normal_texture;\n"
        "uniform sampler2D depth_texture;\n"
      "#endif\n"

      "vec3 lighting(vec3 n_eye, vec3 v_eye, vec3 color, float shininess);\n"
    "#endif\n"

    "in block {\n"
      "vec2 texture_uv;\n"
      "}\n"
    "In;\n"

    "#define FRAG_COLOR 0\n"
    "layout(location = FRAG_COLOR) out vec4 frag_color;\n"

    "void main() {\n"
      "vec4 res = vec4(0.0); \n"
      "#if MULTISAMPLING \n"
        "ivec2 itexture_uv = ivec2(textureSize(color_texture) * In.texture_uv);\n"
        "for (int i = 0; i < 4; ++i)\n"
      "#endif \n"
          "{ \n"
          "#if MULTISAMPLING \n"
            "vec4 pixel = texelFetch(color_texture, itexture_uv, i); \n"

            "#if SMOOTH \n"
              "vec3 normal = normalize(texelFetch(normal_texture, ivec2(itexture_uv), i).xyz); \n"
              "float depth = texelFetch(depth_texture, ivec2(itexture_uv), i).r; \n"
            "#endif\n"
          "#else \n"
            "vec4 pixel = texture(color_texture, In.texture_uv); \n"

            "#if SMOOTH  \n"
              "vec3 normal = normalize(texture(normal_texture, In.texture_uv).xyz);\n"
              "float depth = texture(depth_texture, In.texture_uv).r;\n"
            "#endif\n"
          "#endif\n"

          "if (pixel.a > 0.0) {  \n"
            "#if SMOOTH \n"
              "vec4 p_ndc = vec4 (2.0 * (gl_FragCoord.xy - viewport.xy) / (viewport.zw) - 1.0,\n"
                                "(2.0 * depth - gl_DepthRange.near - gl_DepthRange.far) / gl_DepthRange.diff,\n"
                                "1.0 );\n"

              "vec4 v_eye = projection_matrix_inv * p_ndc;\n"
              "v_eye = v_eye / v_eye.w; \n"

              "res += vec4(lighting(normal, v_eye.xyz, pixel.rgb / pixel.a, material_shininess), 1.0);\n"
            "#else\n"
               "res += vec4(pixel.rgb / pixel.a, 1.0f);\n"
            "#endif\n"
            "}\n"
          "else {\n"
            "res += vec4(1.0);\n"
            "}\n"
          "}\n"

      "#if MULTISAMPLING \n"
        "frag_color = sqrt(res / 4.0);\n"
      "#else\n"
        "frag_color = sqrt(res);\n"
      "#endif\n"
      "}\n";
  //}}}
  //{{{
  const std::string kLightingGlsl =
    "#version 330\n"

    "vec3 lighting(vec3 normal_eye, vec3 v_eye, vec3 color, float shininess) {\n"
      "const vec3 light_eye = vec3(0.0, 0.0, 1.0);\n"
      "float dif = max(dot(light_eye, normal_eye), 0.0);\n"
      "vec3 refl_eye = reflect(light_eye, normal_eye);\n"
      "vec3 view_eye = normalize(v_eye);\n"
      "float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0), shininess);\n"
      "float rim = pow(1.0 + dot(normal_eye, view_eye), 3.0);\n"
      "vec3 res = 0.15 * color;\n"
      "res += 0.6 * dif * color;\n"
      "res += 0.1 * spe * vec3(1.0);\n"
      "res += 0.1 * rim * vec3(1.0);\n"
      "return res;\n"
      "}\n";
  //}}}
  }

//{{{
ProgramAttribute::ProgramAttribute()
    : m_ewa_filter(false), m_backface_culling(false),
      m_visibility_pass(true), m_smooth(false), m_color_material(false),
      m_pointsize_method(0) {

  initialize_shader_obj();
  initialize_program_obj();
  }
//}}}

//{{{
void ProgramAttribute::set_ewa_filter (bool enable) {

  if (m_ewa_filter != enable) {
    m_ewa_filter = enable;
    initialize_program_obj();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_pointsize_method(unsigned int pointsize_method) {

  if (m_pointsize_method != pointsize_method) {
    m_pointsize_method = pointsize_method;
    initialize_program_obj();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_backface_culling(bool enable) {

  if (m_backface_culling != enable) {
    m_backface_culling = enable;
    initialize_program_obj();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_visibility_pass(bool enable) {

  if (m_visibility_pass != enable) {
    m_visibility_pass = enable;
    initialize_program_obj();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_smooth(bool enable) {

  if (m_smooth != enable) {
    m_smooth = enable;
    initialize_program_obj();
    }
  }
//}}}
//{{{
void ProgramAttribute::set_color_material(bool enable) {

  if (m_color_material != enable) {
    m_color_material = enable;
    initialize_program_obj();
    }
  }
//}}}

//{{{
void ProgramAttribute::initialize_shader_obj() {

  m_attribute_vs_obj.load_from_cstr (reinterpret_cast<char const*>(kAttributeVsGlsl.c_str()));
  m_lighting_vs_obj.load_from_cstr (reinterpret_cast<char const*>(kLightingGlsl.c_str()));
  m_attribute_fs_obj.load_from_cstr (reinterpret_cast<char const*>(kAttributeFsGlsl.c_str()));
  }
//}}}
//{{{
void ProgramAttribute::initialize_program_obj() {

  try {
    detach_all();

    attach_shader (m_attribute_vs_obj);
    attach_shader (m_attribute_fs_obj);
    attach_shader (m_lighting_vs_obj);

    map<string, int> defines;
    defines.insert (make_pair ("EWA_FILTER", m_ewa_filter ? 1 : 0));
    defines.insert (make_pair ("BACKFACE_CULLING", m_backface_culling ? 1 : 0));
    defines.insert (make_pair ("VISIBILITY_PASS", m_visibility_pass ? 1 : 0));
    defines.insert (make_pair ("SMOOTH", m_smooth ? 1 : 0));
    defines.insert (make_pair ("COLOR_MATERIAL", m_color_material ? 1 : 0));
    defines.insert (make_pair ("POINTSIZE_METHOD", static_cast<int>(m_pointsize_method)));

    m_attribute_vs_obj.compile (defines);
    m_attribute_fs_obj.compile (defines);
    m_lighting_vs_obj.compile (defines);
    }
  catch (shader_compilation_error const& e) {
    cerr << "Error: A shader failed to compile." << endl << e.what() << endl;
    exit (EXIT_FAILURE);
    }

  try {
    link();
    }
  catch (shader_link_error const& e) {
    cerr << "Error: A program failed to link." << endl << e.what() << endl;
    exit (EXIT_FAILURE);
    }

  try {
    set_uniform_block_binding ("Camera", 0);
    set_uniform_block_binding ("Raycast", 1);
    set_uniform_block_binding ("Frustum", 2);
    set_uniform_block_binding ("Parameter", 3);
    }
  catch (uniform_not_found_error const& e) {
    cerr << "Warning: Failed to set a uniform variable." << endl << e.what() << endl;
    }
  }
//}}}

// ProgramFinalization
//{{{
ProgramFinalization::ProgramFinalization()
    : m_smooth(false), m_multisampling(false) {

  initialize_shader_obj();
  initialize_program_obj();
  }
//}}}

//{{{
void ProgramFinalization::set_multisampling (bool enable) {

  if (m_multisampling != enable) {
    m_multisampling = enable;
    initialize_program_obj();
    }
  }

//}}}
//{{{
void ProgramFinalization::set_smooth (bool enable) {

  if (m_smooth != enable) {
    m_smooth = enable;
    initialize_program_obj();
    }
  }
//}}}

//{{{
void ProgramFinalization::initialize_shader_obj() {

  m_finalization_vs_obj.load_from_cstr (reinterpret_cast<char const*>(kFinalizationVsGlsl.c_str()));
  m_finalization_fs_obj.load_from_cstr (reinterpret_cast<char const*>(kFinalizationFsGlsl.c_str()));
  m_lighting_fs_obj.load_from_cstr (reinterpret_cast<char const*>(kLightingGlsl.c_str()));

  attach_shader (m_finalization_vs_obj);
  attach_shader (m_finalization_fs_obj);
  attach_shader (m_lighting_fs_obj);
  }
//}}}
//{{{
void ProgramFinalization::initialize_program_obj() {

  try {
    map<string, int> defines;
    defines.insert (make_pair ("SMOOTH", m_smooth ? 1 : 0));
    defines.insert (make_pair ("MULTISAMPLING", m_multisampling ? 1 : 0));
    m_finalization_vs_obj.compile (defines);
    m_finalization_fs_obj.compile (defines);
    m_lighting_fs_obj.compile (defines);
    }
  catch (shader_compilation_error const& e) {
    cerr << "Error: A shader failed to compile." << endl << e.what() << endl;
    exit (EXIT_FAILURE);
    }

  try {
    link();
    }
  catch (shader_link_error const& e) {
    cerr << "Error: A program failed to link." << endl << e.what() << endl;
    exit (EXIT_FAILURE);
    }

  try {
    set_uniform_block_binding ("Camera", 0);
    set_uniform_block_binding ("Raycast", 1);
    set_uniform_block_binding ("Parameter", 3);
    }
  catch (uniform_not_found_error const& e) {
    cerr << "Warning: Failed to set a uniform variable." << endl << e.what() << endl;
    }
  }
//}}}
