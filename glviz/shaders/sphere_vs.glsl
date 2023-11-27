#version 330

layout(std140, column_major) uniform Camera {
  mat4 modelview_matrix;
  mat4 modelview_matrix_it;
  mat4 projection_matrix;
  };

layout(std140) uniform Sphere {
  float sphere_radius;
  float projection_radius;
  };

#define ATTR_POSITION 0
layout(location = ATTR_POSITION) in vec3 center;

out block {
  flat vec3 center_eye;
  }
Out;

void main() {
  vec4 center_eye = modelview_matrix * vec4(center, 1.0);
  gl_Position = projection_matrix * center_eye;
  Out.center_eye = vec3(center_eye);
  gl_PointSize = 2.0 * (sphere_radius / -center_eye.z) * projection_radius;
  }
