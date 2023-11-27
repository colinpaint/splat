#version 330
#define SMOOTH  0

layout(std140, column_major) uniform Camera {
  mat4 modelview_matrix;
  mat4 modelview_matrix_it;
  mat4 projection_matrix;
  };

#define ATTR_POSITION 0
layout(location = ATTR_POSITION) in vec3 position;

#if SMOOTH
  #define ATTR_NORMAL 1
  layout(location = ATTR_NORMAL) in vec3 normal;
#endif

out block {
  #if SMOOTH
    vec3 normal;
  #endif
  vec3 position;
  }
Out;

void main() {
  vec4 position_eye = modelview_matrix * vec4(position, 1.0);

  #if SMOOTH
    Out.normal = mat3(modelview_matrix_it) * normal;
  #endif
  Out.position = vec3(position_eye);
  gl_Position = projection_matrix * position_eye;
  }
