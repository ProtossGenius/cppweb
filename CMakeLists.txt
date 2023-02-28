cmake_minimum_required(VERSION 3.8)
project(iceweb)
set(CXX_FLAGS
        -g
        # -DVALGRIND
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
		-fPIC
        )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(. "./third_part/include")
link_directories(. "./third_part/lib")

aux_source_directory(. src_files)
add_library(iceweb ${src_files})

install (TARGETS 	iceweb DESTINATION lib)
install (FILES 		iceweb.h icewebimpl.h run_results.h	DESTINATION include/iceweb)
