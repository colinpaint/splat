cmake_minimum_required (VERSION 3.18)
#
add_subdirectory (glviz)

project (embed VERSION 1.0.0)
  add_executable (${PROJECT_NAME} embed.cpp)

project (example VERSION 1.0.0)
  add_executable (${PROJECT_NAME} example.cpp)
  target_include_directories (${PROJECT_NAME} PRIVATE . eigen)
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
  else()
    find_package (OpenGL REQUIRED)
    target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
  endif()

project (splat VERSION 1.0.0)
  add_executable (${PROJECT_NAME} config.hpp
                                  src/main.cpp
                                  src/framebuffer.cpp
                                  src/program_attribute.cpp
                                  src/program_finalization.cpp
                                  src/splat_renderer.cpp
                                  shaders/lighting_glsl.cpp
                                  shaders/attribute_fs_glsl.cpp
                                  shaders/attribute_vs_glsl.cpp
                                  shaders/finalization_fs_glsl.cpp
                                  shaders/finalization_vs_glsl.cpp
                                  )

  target_include_directories (${PROJECT_NAME} PRIVATE . eigen)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
  else()
    find_package (OpenGL REQUIRED)
    target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
  endif()
