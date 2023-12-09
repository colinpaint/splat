cmake_minimum_required (VERSION 3.18)
#
set (PLATFORM "SDL2" CACHE STRING "sdl version")

add_subdirectory (fmt)
add_subdirectory (common)
add_subdirectory (imgui)
add_subdirectory (app)

#if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
#else()
#  add_subdirectory (objviewer)
#endif()

# splatApp
project (splat VERSION 1.0.0)
  add_executable (${PROJECT_NAME} splat.cpp
                                  cModel.h cModel.cpp
                                  cSurfelModel.h cSurfelModel.cpp
                                  cRender.h
                                  cMeshRender.h cMeshRender.cpp
                                  cSplatRender.h cSplatRender.cpp)
  target_link_libraries (${PROJECT_NAME} PRIVATE app)
