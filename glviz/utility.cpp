//{{{  includes
#include "utility.h"

#include <GL/glew.h>

#include <iostream>
#include <fstream>
#include <Eigen/Dense>

#include "../common/cLog.h"

using namespace std;
//}}}

namespace GLviz {
  //{{{
  string getGlErrorString (GLenum gl_error) {

     string error_string;

     switch (gl_error) {
      case GL_NO_ERROR:
        error_string = "GL_NO_ERROR";
        break;

      case GL_INVALID_ENUM:
        error_string = "GL_INVALID_ENUM";
        break;

      case GL_INVALID_VALUE:
        error_string = "GL_INVALID_VALUE";
        break;

      case GL_INVALID_OPERATION:
        error_string = "GL_INVALID_OPERATION";
        break;

      case GL_INVALID_FRAMEBUFFER_OPERATION:
        error_string = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;

      case GL_OUT_OF_MEMORY:
        error_string = "GL_OUT_OF_MEMORY";
        break;

      default:
        error_string = "UNKNOWN";
      }

    return error_string;
    }
  //}}}
  //{{{
  string getGlFramebufferStatusString (GLenum framebuffer_status) {

    string status_string;

    switch (framebuffer_status) {
      case GL_FRAMEBUFFER_COMPLETE:
        status_string = "GL_FRAMEBUFFER_COMPLETE";
        break;

      case GL_FRAMEBUFFER_UNDEFINED:
        status_string = "GL_FRAMEBUFFER_UNDEFINED";
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        status_string = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        status_string = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        status_string = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        status_string = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;

      case GL_FRAMEBUFFER_UNSUPPORTED:
        status_string = "GL_FRAMEBUFFER_UNSUPPORTED";
        break;

      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        status_string = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;

     case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        status_string = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;

      default:
        status_string = "UNKNOWN";
      }

    return status_string;
    }
  //}}}
  }
