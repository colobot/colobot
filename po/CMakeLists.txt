cmake_minimum_required(VERSION 2.8)

set(_potFile colobot.pot)

find_program(XGETTEXT_CMD xgettext)

add_custom_command(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ${colobot_SOURCE_DIR}/src/app/app.cpp --output=${_potFile}
    COMMAND ${XGETTEXT_CMD} ${colobot_SOURCE_DIR}/src/common/restext.cpp --output=${_potFile} --join-existing --keyword=TR --no-location
    COMMAND sed -i -e "s|^\\(\"POT-Creation-Date:\\).*$|\\1 DATE\\\\n\"|" ${_potFile}

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
    VERBATIM
)

add_custom_target(update-pot DEPENDS ${_potFile})

file(GLOB _poFiles *.po)
set(GETTEXT_INSTALL_PREFIX ${COLOBOT_INSTALL_I18N_DIR})
gettext_create_translations(${_potFile} ALL ${_poFiles})
