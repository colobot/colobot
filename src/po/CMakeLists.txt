cmake_minimum_required(VERSION 2.8)

SET(_potFile colobot.pot)

# TODO: Use a finder
SET(XGETTEXT_CMD /usr/bin/xgettext)

find_package(Gettext REQUIRED)

ADD_CUSTOM_COMMAND(OUTPUT ${_potFile}
    COMMAND ${XGETTEXT_CMD} ../app/app.cpp --output=${_potFile}
    COMMAND ${XGETTEXT_CMD} ../common/restext_strings.c --output=${_potFile} --join-existing --extract-all --no-location

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Extract translatable messages to ${_potFile}"
)

ADD_CUSTOM_TARGET(_${potFile} ${_all}
DEPENDS ${_potFile}
)

FILE(GLOB _poFiles *.po)

GETTEXT_CREATE_TRANSLATIONS(${_potFile} ALL ${_poFiles})
