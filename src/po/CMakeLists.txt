cmake_minimum_required(VERSION 2.8)

set(_potFile colobot.pot)

find_program(XGETTEXT_CMD xgettext)

add_custom_command(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ../app/app.cpp --output=${_potFile}
    COMMAND ${XGETTEXT_CMD} ../common/restext.cpp --output=${_potFile} --join-existing --extract-all --no-location

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
)

add_custom_target(update-pot DEPENDS ${_potFile})

file(GLOB _poFiles *.po)
gettext_create_translations(${_potFile} ALL ${_poFiles})
