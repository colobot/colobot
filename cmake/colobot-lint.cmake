# Some functions for use with colobot-lint

##
# This adds a new target with generated fake source files that include single header files
# It is a workaround for how CMake and Clang handle header files in compilation database
# And we need that to check each header file in the project exactly once, the same as .cpp modules
# Note: This is modifying an existing target by adding fake source files.
##
macro(add_fake_header_sources subdir the_target)
    set(all_fake_header_src_files "")

    set(fake_headers_src_dir ${colobot_BINARY_DIR}/fake_header_sources)
    file(MAKE_DIRECTORY ${fake_headers_src_dir})

    get_target_property(the_target_sources ${the_target} SOURCES)

    foreach(source_file IN LISTS the_target_sources)
        string(REGEX MATCH [[^.*\.h$]] is_header_file ${source_file})
        if(is_header_file)
            file(RELATIVE_PATH header_file ${colobot_SOURCE_DIR} ${colobot_SOURCE_DIR}/${subdir}/${source_file})
            string(REGEX REPLACE "\\.h$" ".cpp" fake_header_src_file "${fake_headers_src_dir}/${header_file}")

            get_filename_component(fake_header_src_dir ${fake_header_src_file} PATH)
            file(MAKE_DIRECTORY ${fake_header_src_dir})

            file(WRITE ${fake_header_src_file} "#include \"${header_file}\"\n\n")

            target_sources(${the_target} PRIVATE ${fake_header_src_file})
            target_include_directories(${the_target} PUBLIC ${colobot_SOURCE_DIR})
        endif()
    endforeach()
endmacro()
