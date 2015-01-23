cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)

macro (define_source_files)
    set (temp_src_dir ${ARGN})
    file (GLOB temp_src_list RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${temp_src_dir}/*.h" "${temp_src_dir}/*.cpp")
    list (APPEND src_files ${temp_src_list})
    unset(temp_src_dir)
    unset(temp_src_list)
endmacro ()

define_source_files (formats)
define_source_files (gl)
define_source_files (log)
define_source_files (math)
define_source_files (memory)
define_source_files (render)
define_source_files (resources)
define_source_files (scene)
define_source_files (system)
define_source_files (ui)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_library(nya_engine ${src_files})
