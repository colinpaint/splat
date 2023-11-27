#pragma once
#include <GL/glew.h>
#include <Eigen/Core>
#include <string>
#include <vector>
#include <array>

namespace GLviz {
  void load_raw (std::string const& filename, std::vector<Eigen::Vector3f>
                 &vertices, std::vector<std::array<unsigned int, 3> >& faces);

  void save_raw (std::string const& filename, std::vector<Eigen::Vector3f>
                 const& vertices, std::vector<std::array<unsigned int, 3> >& faces);

  void set_vertex_normals_from_triangle_mesh (std::vector<Eigen::Vector3f>
    const& vertices, std::vector<std::array<unsigned int, 3> > const& faces,
    std::vector<Eigen::Vector3f>& normals);

  std::string get_gl_error_string (GLenum gl_error);
  std::string get_gl_framebuffer_status_string (GLenum framebuffer_status);
  }
