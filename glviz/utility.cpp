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
  void loadMesh (string const& filename,
                 vector<Eigen::Vector3f>& vertices,
                 vector<array<unsigned int,3>>& faces) {

    ifstream inputFind (filename);
    if (!inputFind.good()) {
      //{{{  error, return
       cLog::log (LOGERROR, fmt::format ("loadMesh - cannot find {}", filename));
      throw runtime_error ("cannot find");
      return;
      }
      //}}}
    inputFind.close();

    ifstream input (filename, ios::in | ios::binary);
    if (input.fail()) {
      ostringstream error_message;
      cLog::log (LOGERROR, fmt::format ("loadMesh - cannot open {}", filename));
      throw runtime_error (error_message.str().c_str());
      }

    unsigned int nv;
    input.read (reinterpret_cast<char*>(&nv), sizeof(unsigned int));
    vertices.resize (nv);

    for (size_t i = 0; i < nv; ++i)
      input.read (reinterpret_cast<char*>(vertices[i].data()), 3 * sizeof(float));

    unsigned int nf;
    input.read (reinterpret_cast<char*>(&nf), sizeof(unsigned int));
    faces.resize (nf);

    for (size_t i = 0; i < nf; ++i)
      input.read (reinterpret_cast<char*>(faces[i].data()), 3 * sizeof(unsigned int));

    input.close();

    cLog::log (LOGINFO, fmt::format ("loadMesh {} vertices:{} faces:{}",
                                     filename, vertices.size(), faces.size()));
    }
  //}}}
  //{{{
  void setVertexNormalsFromTriangleMesh (vector<Eigen::Vector3f> const& vertices,
                                         vector<array<unsigned int,3>> const& faces,
                                         vector<Eigen::Vector3f>& normals) {

    unsigned int nf = static_cast<unsigned int>(faces.size());
    unsigned int nv = static_cast<unsigned int>(vertices.size());

    normals.resize (vertices.size());
    fill (normals.begin(), normals.end(), Eigen::Vector3f::Zero());

    for (size_t i = 0; i < faces.size(); ++i) {
      array<unsigned int, 3> const& f_i = faces[i];

      Eigen::Vector3f const& p0(vertices[f_i[0]]);
      Eigen::Vector3f const& p1(vertices[f_i[1]]);
      Eigen::Vector3f const& p2(vertices[f_i[2]]);

      Eigen::Vector3f n_i = (p0 - p1).cross(p0 - p2);

      normals[f_i[0]] += n_i;
      normals[f_i[1]] += n_i;
      normals[f_i[2]] += n_i;
      }

    for (size_t i = 0; i < vertices.size(); ++i)
      if (!normals[i].isZero())
        normals[i].normalize();
    }
  //}}}

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
