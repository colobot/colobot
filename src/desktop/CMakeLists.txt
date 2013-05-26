cmake_minimum_required(VERSION 2.8)

# Install Desktop Entry file
set(COLOBOT_DESKTOP_FILE colobot.desktop)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${COLOBOT_DESKTOP_FILE}
    COMMAND ./create_desktop_file.sh > ${CMAKE_CURRENT_BINARY_DIR}/${COLOBOT_DESKTOP_FILE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Build ${COLOBOT_DESKTOP_FILE}"
    )
add_custom_target(desktopfile ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${COLOBOT_DESKTOP_FILE})
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${COLOBOT_DESKTOP_FILE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/
    )

# Install Icon
set(COLOBOT_ICON_FILE colobot.svg)
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/${COLOBOT_ICON_FILE}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/
    )

# Render SVG icon in various sizes
find_program(RSVG_CONVERT rsvg-convert)
if(RSVG_CONVERT)
    foreach(PNGSIZE "48" "32" "16")
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${PNGSIZE})
        add_custom_target(resize_icon_${PNGSIZE} ALL
            COMMAND ${RSVG_CONVERT} -w ${PNGSIZE} -h ${PNGSIZE} ${CMAKE_CURRENT_SOURCE_DIR}/${COLOBOT_ICON_FILE}
            > ${CMAKE_CURRENT_BINARY_DIR}/${PNGSIZE}/colobot.png
            )
        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${PNGSIZE}/colobot.png
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${PNGSIZE}x${PNGSIZE}/apps/
            )
    endforeach()
endif()

# Create manpage from pod-formatted file
find_program(POD2MAN pod2man)
if(POD2MAN AND (NOT MSYS))
    set(COLOBOT_MANPAGE_SECTION 6)

        macro(podman)
        cmake_parse_arguments(PM "" "PODFILE;LOCALE;" "" ${ARGN})
        if(PM_LOCALE)
            # This copes with the fact that english has no "/LANG" in the paths and filenames.
            set(SLASHLOCALE /${PM_LOCALE})
        endif()
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE})
        add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PM_PODFILE}
                   COMMAND ${POD2MAN} ARGS --section=${COLOBOT_MANPAGE_SECTION}
                                      --center="Colobot" --stderr --utf8
                                      --release="${COLOBOT_VERSION_FULL}"
                                      ${CMAKE_CURRENT_SOURCE_DIR}/${PM_PODFILE}
                                      ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
           COMMENT "Create ${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION} manpage"
                  )
        add_custom_target(man${PM_LOCALE} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION})

        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man${SLASHLOCALE}/man${COLOBOT_MANPAGE_SECTION}/ )

        add_dependencies(man man${PM_LOCALE})
    endmacro()

    # Create the english manpage
    podman(PODFILE colobot.pod)

endif()

# Translate translatable material
find_program(PO4A po4a)

if(PO4A)
    add_custom_target(desktop_po4a
        COMMAND ${PO4A} po4a.cfg
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_dependencies(desktopfile desktop_po4a)

    if(POD2MAN)
        add_custom_target(man_po4a
            COMMAND ${PO4A} po4a.cfg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_dependencies(man man_po4a)
        file(GLOB LINGUAS_PO RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/po/ ${CMAKE_CURRENT_SOURCE_DIR}/po/*.po)
        string(REGEX REPLACE ".po$" "" LINGUAS ${LINGUAS_PO})
        foreach(LOCALE ${LINGUAS})
            podman(PODFILE lang/${LOCALE}/colobot.pod LOCALE ${LOCALE})
        endforeach()
    endif()
endif()

