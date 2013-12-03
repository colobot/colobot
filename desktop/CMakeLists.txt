cmake_minimum_required(VERSION 2.8)

set(COLOBOT_ICON_FILE colobot.svg)

# Render SVG icon in various sizes
find_program(RSVG_CONVERT rsvg-convert)
if(RSVG_CONVERT AND (PLATFORM_GNU OR PLATFORM_MACOSX))
    add_custom_target(png-icons ALL)
    foreach(PNGSIZE 512 256 128 48 32 16)
        add_custom_command(
            OUTPUT ${PNGSIZE}/colobot.png
            COMMAND mkdir -p ${PNGSIZE}
            COMMAND ${RSVG_CONVERT} -w ${PNGSIZE} -h ${PNGSIZE} ${CMAKE_CURRENT_SOURCE_DIR}/${COLOBOT_ICON_FILE} > ${PNGSIZE}/colobot.png
            )
        add_custom_target(png-icon-${PNGSIZE} ALL DEPENDS ${PNGSIZE}/colobot.png)
        add_dependencies(png-icons png-icon-${PNGSIZE})

        if(PLATFORM_GNU)
            install(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${PNGSIZE}/colobot.png
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${PNGSIZE}x${PNGSIZE}/apps/
                )
        endif()
        # Prepare the ICNS icon generation
        list(APPEND ICNS_SRCS "${PNGSIZE}/colobot.png")
    endforeach()
    
    # Pack icon for Mac OS
    find_program(PNG2ICNS png2icns)
    if(PNG2ICNS AND PLATFORM_MACOSX)
        add_custom_command(OUTPUT Colobot.icns
            COMMAND ${PNG2ICNS} Colobot.icns ${ICNS_SRCS}
            DEPENDS png-icons
            )
        add_custom_target(icns-icon ALL DEPENDS Colobot.icns)
    endif()

endif()

if(PLATFORM_GNU)
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
    install(
        FILES ${CMAKE_CURRENT_SOURCE_DIR}/${COLOBOT_ICON_FILE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/
        )

    # Translate translatable material
    find_program(PO4A po4a)
    if(PO4A)
        add_custom_target(desktop_po4a
            COMMAND ${PO4A} po4a.cfg
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
        add_dependencies(desktopfile desktop_po4a)
    endif()

    # Create manpage from pod-formatted file
    find_program(POD2MAN pod2man)
    if(POD2MAN)
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

        if(PO4A)
            # Translate the manpage to other languages
            add_dependencies(man desktop_po4a)
            file(GLOB LINGUAS_PO RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/po/ ${CMAKE_CURRENT_SOURCE_DIR}/po/*.po)
            string(REGEX REPLACE ".po$" "" LINGUAS ${LINGUAS_PO})
            foreach(LOCALE ${LINGUAS})
                podman(PODFILE lang/${LOCALE}/colobot.pod LOCALE ${LOCALE})
                add_dependencies(man${PM_LOCALE} desktop_po4a)
            endforeach()
        endif()
    endif()
endif(PLATFORM_GNU)

if(PLATFORM_MACOSX)
    configure_file(Info.plist.cmake ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
endif(PLATFORM_MACOSX)

if(PLATFORM_WINDOWS)
    set(COLOBOT_VERSION_4COMMAS "${COLOBOT_VERSION_MAJOR},${COLOBOT_VERSION_MINOR},${COLOBOT_VERSION_REVISION},0")
    configure_file(colobot.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/colobot.rc)
endif(PLATFORM_WINDOWS)


##
# Packaging
##
set(CPACK_BUNDLE_NAME "Colobot")
set(CPACK_PACKAGE_FILE_NAME "colobot-${COLOBOT_VERSION_FULL}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Colobot - Colonize with Bots")
set(CPACK_PACKAGE_VENDOR "Polish Portal of Colobot")

set(CPACK_PACKAGE_VERSION_MAJOR ${COLOBOT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${COLOBOT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${COLOBOT_VERSION_REVISION})

if(PLATFORM_WINDOWS)
    set(CPACK_STRIP_FILES TRUE)

    # Don't version the install directory, and allow overwriting
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Colobot")
    set(CPACK_NSIS_DEFINES "SetOverwrite on\nBrandingText \\\"Colobot ${COLOBOT_VERSION_CODENAME} (${COLOBOT_VERSION_FULL})\\\"")
    # Install the executable directly in Program Files/Colobot/
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_PACKAGE_EXECUTABLES "colobot" "Colobot ${COLOBOT_VERSION_CODENAME}")

    # Branding
    ## Installation and uninstallation icons
    set(CPACK_NSIS_MUI_ICON    ${CMAKE_CURRENT_SOURCE_DIR}/colobot.ico)
    set(CPACK_NSIS_MUI_UNIICON ${CMAKE_CURRENT_SOURCE_DIR}/colobot.ico)
    ## Header and Left banner images
    set(CPACK_PACKAGE_ICON     "")
    set(CPACK_NSIS_INSTALLER_ICON_CODE    "!define MUI_HEADERIMAGE_BITMAP       \\\"${CMAKE_CURRENT_SOURCE_DIR}/colobot_nsis.bmp\\\"
                                           !define MUI_WELCOMEFINISHPAGE_BITMAP \\\"${CMAKE_CURRENT_SOURCE_DIR}/colobot_nsis_left.bmp\\\"")

    ## Allow to directly run Colobot at the end of the installation
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN "colobot.exe")

    ## TODO: Find a way to have no license prompt. Until that, display the GPL-3
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.txt)

elseif(PLATFORM_MACOSX)
    set(CPACK_GENERATOR "Bundle")
    set(CPACK_BUNDLE_ICON  ${CMAKE_CURRENT_BINARY_DIR}/Colobot.icns)
    set(CPACK_PACKAGE_ICON ${CMAKE_CURRENT_BINARY_DIR}/Colobot.icns)
    set(CPACK_BUNDLE_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)

    set(CPACK_BINARY_DRAGNDROP ON)

    install(CODE "
        include(BundleUtilities)
        message(STATUS \"fixup_bundle(\${CMAKE_INSTALL_PREFIX}/${COLOBOT_INSTALL_BIN_DIR}/colobot/\")
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${COLOBOT_INSTALL_BIN_DIR}/colobot\"   \"\"   \"\")
        ")
endif()

include(CPack)
