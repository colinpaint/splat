#version 330
#define SMOOTH     0
#define WIREFRAME  0

layout(std140, column_major) uniform Camera {
  mat4 modelview_matrix;
  mat4 modelview_matrix_it;
  mat4 projection_matrix;
  };

#if WIREFRAME
  layout(std140) uniform Wireframe {
    vec3 color_wireframe;
    ivec2 viewport;
    };
#endif

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in block {
  #if SMOOTH
    vec3 normal;
  #endif
  vec3 position;
  }
In[];

out block {
  #if SMOOTH
    vec3 normal;
  #else
    flat vec3 normal;
  #endif

  vec3 position;

  #if WIREFRAME
    noperspective vec3 distance;
  #endif
  }
Out;

void main() {
  #if WIREFRAME
    vec2 w0 = (1.0 / gl_in[0].gl_Position.w) * gl_in[0].gl_Position.xy * viewport.xy;
    vec2 w1 = (1.0 / gl_in[1].gl_Position.w) * gl_in[1].gl_Position.xy * viewport.xy;
    vec2 w2 = (1.0 / gl_in[2].gl_Position.w) * gl_in[2].gl_Position.xy * viewport.xy;
    mat3 matA = mat3(vec3(1.0, w0), vec3(1.0, w1), vec3(1.0, w2));
    float area = abs(determinant(matA));
  #endif

  #if !SMOOTH
    vec3 normal = normalize(cross(In[1].position - In[0].position, In[2].position - In[0].position));
  #endif

  gl_Position = gl_in[0].gl_Position;
  #if SMOOTH
    Out.normal = In[0].normal;
  #else
    Out.normal = normal;
  #endif
  Out.position = In[0].position;

  #if WIREFRAME
    Out.distance = vec3(area / length(w2 - w1), 0.0, 0.0);
  #endif
  EmitVertex();

  gl_Position = gl_in[1].gl_Position;
  #if SMOOTH
    Out.normal = In[1].normal;
  #else
    Out.normal = normal;
  #endif

  Out.position = In[1].position;
  #if WIREFRAME
    Out.distance = vec3(0.0, area / length(w2 - w0), 0.0);
  #endif
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  #if SMOOTH
    Out.normal = In[2].normal;
  #else
    Out.normal = normal;
  #endif
  Out.position = In[2].position;
  #if WIREFRAME
    Out.distance = vec3(0.0, 0.0, area / length(w1 - w0));
  #endif
  EmitVertex();
  }
