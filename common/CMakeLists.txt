project (common VERSION 1.0.0)
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    add_library (${PROJECT_NAME} date.h cLog.h cLog.cpp)
  else()
    add_library (${PROJECT_NAME} date.h cLog.h cLog.cpp)
  endif()

  target_link_libraries (${PROJECT_NAME} PUBLIC fmt)

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_compile_options (${PROJECT_NAME} PUBLIC /W3  # /W4 Eigen problems
                                                   ${BUILD_MSVC_COMPILE_OPTIONS})
    if (USE_AVX512)
      message (STATUS "windows built with AVX X2 AVX512")
      target_compile_options (${PROJECT_NAME} PUBLIC /arch:AVX512)
    elseif (USE_AVX2)
      message (STATUS "windows built with AVX AVX2")
      target_compile_options (${PROJECT_NAME} PUBLIC /arch:AVX2)
    else()
      message (STATUS "windows built with AVX")
    endif()

  else()
    target_compile_definitions (${PROJECT_NAME} PUBLIC _LARGEFILE64_SOURCE _FILE_OFFSET_BITS=64)
    target_compile_options (${PROJECT_NAME} PUBLIC -g
                                                   -Wextra
                                                   -Wno-missing-field-initializers
                                                   -Wno-format-security
                                                   -Wno-format-overflow
                                                   -pedantic
                                                   -flax-vector-conversions
                                                   -ftree-vectorize
                                                   #-march=native
                                                   -g
                                                   ${BUILD_LINUX_COMPILE_OPTIONS})
    message (STATUS "linux built with native")
  endif()
