cmake_minimum_required (VERSION 3.18)
#
add_subdirectory (imgui)
add_subdirectory (glviz)

# embed file as bin
project (embed VERSION 1.0.0)
  add_executable (${PROJECT_NAME} embed.cpp)

# simple dragon
project (simple VERSION 1.0.0)
  add_executable (${PROJECT_NAME} simple.cpp)
  target_link_libraries (${PROJECT_NAME} PRIVATE glviz)

# splat dragon
project (splat VERSION 1.0.0)
  add_executable (${PROJECT_NAME} splat.cpp
                                  framebuffer.h framebuffer.cpp
                                  program_attribute.h program_attribute.cpp
                                  program_finalization.h program_finalization.cpp
                                  splatRenderer.h splatRenderer.cpp
                                  shaders/attribute_fs_glsl.cpp shaders/attribute_vs_glsl.cpp
                                  shaders/finalization_fs_glsl.cpp shaders/finalization_vs_glsl.cpp)

  target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
