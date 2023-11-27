project (glviz VERSION 1.0.0)
  add_library (${PROJECT_NAME} glviz.h glviz.cpp
                               camera.h camera.cpp
                               shader.h shader.cpp
                               trackball.h trackball.cpp
                               program.h program.cpp
                               buffer.h buffer.cpp
                               utility.h utility.cpp
                               shaders/mesh3_gs_glsl.cpp shaders/mesh3_vs_glsl.cpp shaders/mesh3_fs_glsl.cpp
                               shaders/sphere_fs_glsl.cpp shaders/sphere_vs_glsl.cpp
                               )
  # for Eigen includes
  target_include_directories (${PROJECT_NAME} PUBLIC ../)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_link_libraries (${PROJECT_NAME} PRIVATE opengl32)
  else()
    find_package (OpenGL REQUIRED)
    find_package (SDL2 REQUIRED CONFIG)
    target_link_libraries (${PROJECT_NAME} PUBLIC imgui SDL2::SDL2main SDL2::SDL2-static GL -lGLEW pthread)
  endif()
