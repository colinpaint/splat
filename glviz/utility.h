#pragma once
#include <GL/glew.h>
#include <Eigen/Core>
#include <string>
#include <vector>
#include <array>

namespace GLviz {
  std::string getGlErrorString (GLenum gl_error);
  std::string getGlFramebufferStatusString (GLenum framebuffer_status);
  }
