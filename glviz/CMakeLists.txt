project (glviz VERSION 1.0.0)
  add_library (${PROJECT_NAME} glviz.h glviz.cpp
                               camera.h camera.cpp
                               shader.h shader.cpp
                               trackball.h trackball.cpp
                               program.h program.cpp
                               buffer.h buffer.cpp
                               utility.h utility.cpp)
  # for Eigen includes
  target_include_directories (${PROJECT_NAME} PUBLIC ../ ../glew/include ../imgui)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_compile_definitions (${PROJECT_NAME} PUBLIC GLEW_STATIC=1)
    target_include_directories (${PROJECT_NAME} PUBLIC ../SDL2/include)
    target_link_libraries (${PROJECT_NAME} PRIVATE imgui ../SDL2/SDL2-static ../SDL2/SDL2main
                                                   opengl32 ../glew/glew32s winmm Setupapi version)
  else()
    find_package (OpenGL REQUIRED)
    find_package (SDL2 REQUIRED CONFIG)
    target_link_libraries (${PROJECT_NAME} PUBLIC imgui SDL2::SDL2main SDL2::SDL2-static
                                                        GL -lGLEW pthread)
  endif()
