//{{{  includes
#include "program_finalization.h"

#include <iostream>
#include <cstdlib>

extern unsigned char const finalization_vs_glsl[];
extern unsigned char const finalization_fs_glsl[];
extern unsigned char const lighting_glsl[];

using namespace std;
//}}}

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

  m_finalization_vs_obj.load_from_cstr (reinterpret_cast<char const*>(finalization_vs_glsl));
  m_finalization_fs_obj.load_from_cstr (reinterpret_cast<char const*>(finalization_fs_glsl));
  m_lighting_fs_obj.load_from_cstr (reinterpret_cast<char const*>(lighting_glsl));

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
