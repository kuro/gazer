cmake_minimum_required(VERSION 2.6)

file(READ media/shaders/tonemap.cg tonemap_shader_source)
string(
    REGEX REPLACE
    "\""
    "\\\\\""
    tonemap_shader_source
    "${tonemap_shader_source}"
    )
string(
    REGEX REPLACE
    "([^\n]*)\n"
    "\"\\1\\\\n\"\n"
    tonemap_shader_source
    "${tonemap_shader_source}"
    )
configure_file(src/shaders.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/shaders.h)

subdirs(src)
