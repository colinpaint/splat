project (glviz LANGUAGES CXX)
  add_library (${PROJECT_NAME} src/glviz.hpp
                     src/glviz.cpp
                     src/camera.hpp
                     src/camera.cpp
                     src/shader.cpp
                     src/trackball.hpp
                     src/trackball.cpp
                     src/program.hpp
                     src/program.cpp
                     src/buffer.hpp
                     src/buffer.cpp
                     src/utility.cpp
                     src/utility.hpp
                     src/shader.hpp
                     shaders/mesh3_gs_glsl.cpp
                     shaders/mesh3_vs_glsl.cpp
                     shaders/mesh3_fs_glsl.cpp
                     shaders/sphere_fs_glsl.cpp
                     shaders/sphere_vs_glsl.cpp
                     )

  find_package (SDL2 REQUIRED CONFIG)
  find_package (Eigen3 REQUIRED CONFIG)

  add_subdirectory (imgui)

  target_include_directories (${PROJECT_NAME} PUBLIC /include ./imgui)
  target_link_libraries (${PROJECT_NAME} PUBLIC imgui
                                                Eigen3::Eigen
                                                SDL2::SDL2main SDL2::SDL2-static
                                                -lGLEW
                                                )

  find_package (OpenGL REQUIRED)
  target_link_libraries (${PROJECT_NAME} PRIVATE imgui GL pthread)
