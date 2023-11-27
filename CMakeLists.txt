cmake_minimum_required (VERSION 3.18)
#sudo apt install libglew-dev
#sudo apt install eigen-dev
#
add_subdirectory (glviz)

project (embed VERSION 1.0.0)
  add_executable (${PROJECT_NAME} embed.cpp)

project (example VERSION 1.0.0)
  add_executable (${PROJECT_NAME} example.cpp)
  target_include_directories (${PROJECT_NAME} PRIVATE . eigen)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  else()
    find_package (OpenGL REQUIRED)
  endif()

  target_link_libraries (${PROJECT_NAME} PRIVATE glviz)

project (splat VERSION 1.0.0)
  add_executable (${PROJECT_NAME} splat.cpp
                                  framebuffer.h framebuffer.cpp
                                  program_attribute.h program_attribute.cpp
                                  program_finalization.h program_finalization.cpp
                                  splat_renderer.h splat_renderer.cpp
                                  shaders/lighting_glsl.cpp
                                  shaders/attribute_fs_glsl.cpp
                                  shaders/attribute_vs_glsl.cpp
                                  shaders/finalization_fs_glsl.cpp
                                  shaders/finalization_vs_glsl.cpp
                                  )

  target_include_directories (${PROJECT_NAME} PRIVATE . eigen)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  else()
    find_package (OpenGL REQUIRED)
  endif()

  target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
