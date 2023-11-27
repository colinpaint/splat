#pragma once
#include "glviz/program.h"

class ProgramAttribute : public glProgram {
public:
  ProgramAttribute();

  void set_ewa_filter (bool enable = true);
  void set_pointsize_method (unsigned int pointsize_method);
  void set_backface_culling (bool enable = true);
  void set_visibility_pass (bool enable = true);
  void set_smooth (bool enable = true);
  void set_color_material (bool enable = true);

private:
  void initialize_shader_obj();
  void initialize_program_obj();

  glVertexShader m_attribute_vs_obj, m_lighting_vs_obj;
  glFragmentShader m_attribute_fs_obj;

  bool m_ewa_filter;
  bool m_backface_culling;
  bool m_visibility_pass;
  bool m_smooth;
  bool m_color_material;
  unsigned int m_pointsize_method;
  };

class ProgramFinalization : public glProgram {
public:
  ProgramFinalization();

  void set_multisampling(bool enable);
  void set_smooth(bool enable);

private:
  void initialize_shader_obj();
  void initialize_program_obj();

  glVertexShader    m_finalization_vs_obj;
  glFragmentShader  m_finalization_fs_obj, m_lighting_fs_obj;

  bool m_smooth, m_multisampling;
  };
