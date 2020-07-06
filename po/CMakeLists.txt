cmake_minimum_required(VERSION 2.8)

find_package(Gettext REQUIRED)

set(_potFile colobot.pot)

# Extract translations

find_program(XGETTEXT_CMD xgettext REQUIRED)

# xgettext changes the POT-Creation-Date even when there were no changes,
# which is annoying for version control,
# so replace this date with a constant string
set(POT_CREATION_DATE_REGEX_MATCH "^(\"POT-Creation-Date:).*$")
set(POT_CREATION_DATE_REGEX_REPLACE "\\1 DATE\\\\n\"")

add_custom_command(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ${colobot_SOURCE_DIR}/src/app/app.cpp --output=${_potFile} --no-wrap
    COMMAND ${XGETTEXT_CMD} ${colobot_SOURCE_DIR}/src/common/restext.cpp --output=${_potFile} --no-wrap --join-existing --no-location --keyword=TR
    COMMAND ${XGETTEXT_CMD} ${colobot_SOURCE_DIR}/src/script/script.cpp --output=${_potFile} --no-wrap --join-existing --no-location
    COMMAND ${CMAKE_COMMAND} -DREGEX_MATCH=${POT_CREATION_DATE_REGEX_MATCH} -DREPLACE_WITH=${POT_CREATION_DATE_REGEX_REPLACE} -DFILE_PATH=${_potFile} -P ${colobot_SOURCE_DIR}/cmake/replace_in_file.cmake

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
    VERBATIM
)

add_custom_target(update-pot DEPENDS ${_potFile})

# Generate translations

file(GLOB _poFiles *.po)

set(_gmoFiles)
get_filename_component(_potName ${_potFile} NAME)
string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" _potBasename ${_potName})
get_filename_component(_absPotFile ${_potFile} ABSOLUTE)

foreach (_currentPoFile ${_poFiles})
    get_filename_component(_absFile ${_currentPoFile} ABSOLUTE)
    get_filename_component(_abs_PATH ${_absFile} PATH)
    get_filename_component(_lang ${_absFile} NAME_WE)
    set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

    add_custom_command(
        OUTPUT ${_gmoFile}
        COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} --quiet --no-wrap --update --backup=none -s ${_absFile} ${_absPotFile}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE}  --check-format -o ${_gmoFile} ${_absFile}
        DEPENDS ${_absPotFile} ${_absFile}
    )

    install(FILES ${_gmoFile} DESTINATION ${COLOBOT_INSTALL_I18N_DIR}/${_lang}/LC_MESSAGES RENAME ${_potBasename}.mo)
    set(_gmoFiles ${_gmoFiles} ${_gmoFile})
endforeach()

add_custom_target(translations ALL DEPENDS ${_gmoFiles})
