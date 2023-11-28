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
  void initShader();
  void initProgram();

  glVertexShader m_attribute_vs_obj;
  glVertexShader m_lighting_vs_obj;
  glFragmentShader m_attribute_fs_obj;

  bool m_ewa_filter;
  bool m_backface_culling;
  bool m_visibility_pass;
  bool m_smooth;
  bool m_color_material;
  unsigned int m_pointsize_method;
  };

class ProgramFinal : public glProgram {
public:
  ProgramFinal();

  void set_multisampling (bool enable);
  void set_smooth (bool enable);

private:
  void initShader();
  void initProgram();

  glVertexShader m_Final_vs_obj;
  glFragmentShader m_Final_fs_obj;
  glFragmentShader m_lighting_fs_obj;

  bool m_smooth, m_multisampling;
  };
