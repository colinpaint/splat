project (imgui VERSION 1.0.0)
  file (GLOB HEADER_FILES *.h)
  file (GLOB SOURCE_FILES *.cpp)
  add_library (${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES}
                               backends/imgui_impl_opengl3_loader.h
                               backends/imgui_impl_opengl3.cpp
                               backends/imgui_impl_sld2.h
                               backends/imgui_impl_sdl2.cpp)

  target_include_directories (${PROJECT_NAME} PUBLIC .)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_include_directories (${PROJECT_NAME} PRIVATE ../SDL2/include ../glew/include)
    target_compile_definitions (${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW=1)
  else()
    find_package (SDL2 REQUIRED CONFIG)
    target_include_directories (${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
  endif()
