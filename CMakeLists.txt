cmake_minimum_required (VERSION 3.18)
#
add_subdirectory (fmt)
add_subdirectory (common)
add_subdirectory (imgui)
add_subdirectory (glviz)

# embed file as bin
#project (embed VERSION 1.0.0)
#  add_executable (${PROJECT_NAME} tools/embed.cpp)

# simple dragon
project (simple VERSION 1.0.0)
  add_executable (${PROJECT_NAME} simple.cpp)
  target_link_libraries (${PROJECT_NAME} PRIVATE glviz)

# splat dragon
project (splat VERSION 1.0.0)
  add_executable (${PROJECT_NAME} splat.cpp
                                  framebuffer.h framebuffer.cpp
                                  splatProgram.h splatProgram.cpp
                                  splatRenderer.h splatRenderer.cpp)

  target_link_libraries (${PROJECT_NAME} PRIVATE glviz)
