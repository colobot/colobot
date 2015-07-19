# Some functions for use with colobot-lint

##
# This adds a new target with generated fake source files that include single header files
# It is a workaround for how CMake and Clang handle header files in compilation database
# And we need that to check each header file in the project exactly once, the same as .cpp modules
##
macro(add_fake_header_sources subdir)
    set(all_fake_header_src_files "")

    set(fake_headers_src_dir ${colobot_BINARY_DIR}/fake_header_sources)
    file(MAKE_DIRECTORY ${fake_headers_src_dir})

    file(GLOB_RECURSE all_header_files RELATIVE ${colobot_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

    foreach(header_file ${all_header_files})
        string(REGEX REPLACE "\\.h$" ".cpp" fake_header_src_file "${fake_headers_src_dir}/${header_file}")

        get_filename_component(fake_header_src_dir ${fake_header_src_file} PATH)
        file(MAKE_DIRECTORY ${fake_header_src_dir})

        file(WRITE ${fake_header_src_file} "#include \"${header_file}\"\n\n")

        list(APPEND all_fake_header_src_files ${fake_header_src_file})
    endforeach()

    include_directories(${colobot_SOURCE_DIR})
    add_library(colobot_${subdir}_fake_header_srcs STATIC ${all_fake_header_src_files})
endmacro()
