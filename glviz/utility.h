#pragma once
#include <GL/glew.h>
#include <Eigen/Core>
#include <string>
#include <vector>
#include <array>

namespace GLviz {
  void saveRaw (std::string const& filename, std::vector<Eigen::Vector3f> const& vertices,
                std::vector<std::array<unsigned int, 3> >& faces);
  void loadRaw (std::string const& filename, std::vector<Eigen::Vector3f> &vertices,
                std::vector<std::array<unsigned int, 3> >& faces);
  void loadMesh (std::string const& filename,
                 std::vector<Eigen::Vector3f>& vertices,
                 std::vector<std::array<unsigned int, 3>>& faces);

  void setVertexNormalsFromTriangleMesh (std::vector<Eigen::Vector3f> const& vertices,
                                         std::vector<std::array<unsigned int, 3>> const& faces,
                                         std::vector<Eigen::Vector3f>& normals);

  std::string getGlErrorString (GLenum gl_error);
  std::string getGlFramebufferStatusString (GLenum framebuffer_status);
  }
