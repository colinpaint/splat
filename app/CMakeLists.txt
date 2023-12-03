project (app VERSION 1.0.0)
  add_library (${PROJECT_NAME} cApp.h cApp.cpp
                               cCamera.h cCamera.cpp
                               shaders.h shaders.cpp
                               baseClasses.h baseClasses.cpp)

  target_include_directories (${PROJECT_NAME} PUBLIC ../ ../glew/include ../imgui)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    if (BUILD_SDL_VERSION STREQUAL "SDL3")
      target_compile_definitions (${PROJECT_NAME} PUBLIC GLEW_STATIC=1 $(BUILD_SDL_VERSION))
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL3/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE ../SDL3/SDL3
                                                     opengl32 ../glew/glew32s winmm Setupapi version)
    else()
      target_compile_definitions (${PROJECT_NAME} PUBLIC GLEW_STATIC=1)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL2/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE ../SDL2/SDL2-static ../SDL2/SDL2main
                                                   opengl32 ../glew/glew32s winmm Setupapi version)
    endif()

  else()
    find_package (OpenGL REQUIRED)

    target_compile_definitions (${PROJECT_NAME} PRIVATE $(BUILD_SDL_VERSION))
    if (BUILD_SDL_VERSION STREQUAL "SDL3")
      find_package (SDL3 REQUIRED CONFIG)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL3/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE SDL3::SDL3 GL -lGLEW pthread)
    else()
      find_package (SDL2 REQUIRED CONFIG)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL2/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2-static GL -lGLEW pthread)
    endif()
  endif()

  target_link_libraries (${PROJECT_NAME} PUBLIC imgui)
