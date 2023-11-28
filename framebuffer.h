#pragma once
#include <GL/glew.h>
#include <memory>

class Framebuffer {
public:
  Framebuffer();
  ~Framebuffer();

  GLuint color_texture();

  void enable_depth_texture();
  void disable_depth_texture();
  GLuint depth_texture();

  void attach_normal_texture();
  void detach_normal_texture();
  GLuint normal_texture();

  void set_multisample (bool enable = true);

  void bind();
  void unbind();
  void resize (GLint width, GLint height);

private:
  void initialize();
  void remove_and_delete_attachments();

  GLuint m_fbo;
  GLuint m_color;
  GLuint m_normal;
  GLuint m_depth;

  struct Impl;
  struct Default;
  struct Multisample;
  std::unique_ptr<Impl> m_pimpl;
  };
