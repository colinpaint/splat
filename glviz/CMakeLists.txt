project (glviz LANGUAGES CXX)
  add_library (${PROJECT_NAME} glviz.hpp glviz.cpp
                               camera.hpp camera.cpp
                               shader.hpp
                               shader.cpp
                               trackball.hpp trackball.cpp
                               program.hpp program.cpp
                               buffer.hpp buffer.cpp
                               utility.cpp utility.hpp
                               shaders/mesh3_gs_glsl.cpp
                               shaders/mesh3_vs_glsl.cpp
                               shaders/mesh3_fs_glsl.cpp
                               shaders/sphere_fs_glsl.cpp
                               shaders/sphere_vs_glsl.cpp
                               )

  add_subdirectory (imgui)

  target_include_directories (${PROJECT_NAME} PUBLIC . ./imgui)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_link_libraries (${PROJECT_NAME} PRIVATE imgui opengl32)

  else()
    find_package (SDL2 REQUIRED CONFIG)
    find_package (Eigen3 REQUIRED CONFIG)
    find_package (OpenGL REQUIRED)
    target_link_libraries (${PROJECT_NAME} PUBLIC imgui
                                                  Eigen3::Eigen
                                                  SDL2::SDL2main SDL2::SDL2-static
                                                  GL
                                                  -lGLEW
                                                  pthread)
  endif()
