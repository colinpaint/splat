#version 330

#define ATTR_POSITION 0
layout(location = ATTR_POSITION) in vec3 position;

#define ATTR_TEXTURE_UV 1
layout(location = ATTR_TEXTURE_UV) in vec2 texture_uv;

out block {
  vec2 texture_uv;
  }
Out;

void main() {
  gl_Position = vec4(position, 1.0);
  Out.texture_uv = texture_uv;
  }
