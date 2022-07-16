find_package (GLviz REQUIRED CONFIG)

file (TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/resources/" GLVIZ_RESOURCES_DIR)
configure_file (config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# shader
set (SHADER_GLSL attribute_fs.glsl attribute_vs.glsl
                 finalization_fs.glsl finalization_vs.glsl
                 lighting.glsl
                 )

include (GLvizShaderWrapCpp)
glviz_shader_wrap_cpp (SHADER_CPP ${SHADER_GLSL})

# shader library
add_library (shader STATIC)
target_sources (shader PRIVATE ${SHADER_GLSL} ${SHADER_CPP} )
source_group ("Shader Files" FILES ${SHADER_GLSL})

# splat exe
add_executable (splat main.cpp
                      framebuffer.h
                      framebuffer.cpp
                      program_finalization.h
                      program_finalization.cpp
                      program_attribute.h
                      program_attribute.cpp
                      splat_renderer.cpp
                      splat_renderer.h
                      )
target_include_directories (splat PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> )
target_link_libraries (splat PRIVATE shader GLviz::glviz )

