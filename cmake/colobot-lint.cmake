# Some functions for use with colobot-lint

option(COLOBOT_LINT_BUILD "Enable some additional functions for use with colobot-lint")

if(COLOBOT_LINT_BUILD)

    set(all_fake_header_src_files "")

    set(fake_headers_src_dir ${colobot_BINARY_DIR}/fake_header_sources)
    file(MAKE_DIRECTORY ${fake_headers_src_dir})

    file(GLOB_RECURSE all_header_files RELATIVE ${colobot_SOURCE_DIR} src/*.h test/*.h)

    foreach(header_file ${all_header_files})
        string(REGEX REPLACE "\\.h$" ".cpp" fake_header_src_file "${fake_headers_src_dir}/${header_file}")

        get_filename_component(fake_header_src_dir ${fake_header_src_file} PATH)
        file(MAKE_DIRECTORY ${fake_header_src_dir})

        file(WRITE ${fake_header_src_file} "#include \"${header_file}\"\n\n")

        list(APPEND all_fake_header_src_files ${fake_header_src_file})
    endforeach()

    include_directories(${colobot_SOURCE_DIR})
    add_library(colobot_fake_header_srcs STATIC ${all_fake_header_src_files})

endif()
