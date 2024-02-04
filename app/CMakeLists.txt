project (app VERSION 1.0.0)
  add_library (${PROJECT_NAME} cApp.h cApp.cpp
                               cCamera.h cCamera.cpp
                               baseClasses.h baseClasses.cpp)

  target_include_directories (${PROJECT_NAME} PUBLIC ../ ../glew/include ../imgui)

  if (CMAKE_HOST_WIN32)
    if (PLATFORM STREQUAL "SDL3")
      message (STATUS "building ${PLATFORM} SDL3 for windows")
      target_compile_definitions (${PROJECT_NAME} PUBLIC GLEW_STATIC=1 $PLATFORM)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL3/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE ../SDL3/SDL3
                                                     opengl32 ../glew/glew32s winmm Setupapi version)
    else()
      message (STATUS "building ${PLATFORM} SDL2 for windows")
      target_compile_definitions (${PROJECT_NAME} PUBLIC GLEW_STATIC=1)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL2/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE ../SDL2/SDL2-static ../SDL2/SDL2main
                                                     opengl32 ../glew/glew32s winmm Setupapi version)
    endif()

  else()
    find_package (OpenGL REQUIRED)

    target_compile_definitions (${PROJECT_NAME} PRIVATE $PLATFORM)
    if (PLATFORM STREQUAL "SDL3")
      message (STATUS "building ${PLATFORM} SDL3 for linux")
      find_package (SDL3 REQUIRED CONFIG)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL3/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE SDL3::SDL3 GL -lGLEW pthread)
    else()
      message (STATUS "building ${PLATFORM} SDL2 for linux")
      find_package (SDL2 REQUIRED CONFIG)
      target_include_directories (${PROJECT_NAME} PUBLIC ../SDL2/include)
      target_link_libraries (${PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2-static GL -lGLEW pthread)
    endif()
  endif()

  target_link_libraries (${PROJECT_NAME} PUBLIC common imgui)
