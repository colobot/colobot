cmake_minimum_required(VERSION 3.16)

set(COLOBOT_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/colobot.svg)

# Render SVG icon in various sizes
find_program(RSVG_CONVERT rsvg-convert)
if(NOT RSVG_CONVERT)
    message(STATUS "rsvg-convert not found; desktop icons will not be generated")
endif()

find_program(XMLSTARLET xmlstarlet)
if(NOT XMLSTARLET)
    message(STATUS "xmlstarlet not found; desktop icons will not be generated")
endif()

if(RSVG_CONVERT AND XMLSTARLET AND (PLATFORM_GNU OR PLATFORM_MACOSX))
    add_custom_target(png-icons ALL DEPENDS ${COLOBOT_ICON_FILE})

    foreach(PNG_SIZE 512 256 128 48 32 16)
        # Using xmlstarlet to edit SVG file is a workaround for rsvg-convert bug (see: https://bugzilla.gnome.org/show_bug.cgi?id=762115)
        add_custom_command(
            OUTPUT ${PNG_SIZE}/colobot.png
            COMMAND mkdir -p ${PNG_SIZE}
            COMMAND ${XMLSTARLET} ed -u /*/@width -v ${PNG_SIZE} -u /*/@height -v ${PNG_SIZE} ${COLOBOT_ICON_FILE} > ${PNG_SIZE}/colobot.svg
            COMMAND ${RSVG_CONVERT} ${PNG_SIZE}/colobot.svg -o ${PNG_SIZE}/colobot.png
            )
        add_custom_target(png-icon-${PNG_SIZE} ALL DEPENDS ${PNG_SIZE}/colobot.png)
        add_dependencies(png-icons png-icon-${PNG_SIZE})

        if(PLATFORM_GNU)
            install(
                FILES ${CMAKE_CURRENT_BINARY_DIR}/${PNG_SIZE}/colobot.png
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/${PNG_SIZE}x${PNG_SIZE}/apps/
                )
        endif()
        # Prepare the ICNS icon generation
        list(APPEND ICNS_SRCS "${PNG_SIZE}/colobot.png")
    endforeach()

    # Pack icon for Mac OS
    if(PLATFORM_MACOSX)
        find_program(PNG2ICNS png2icns)
        if(NOT PNG2ICNS)
            message(WARNING "png2icns not found; icns file will not be generated")
        endif()

        if(PNG2ICNS)
            add_custom_command(OUTPUT Colobot.icns
                COMMAND ${PNG2ICNS} Colobot.icns ${ICNS_SRCS}
                DEPENDS png-icons
                )
            add_custom_target(icns-icon ALL DEPENDS Colobot.icns)
        endif()
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

    # Install appdata
    install(
        FILES info.colobot.Colobot.appdata.xml
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo/
        )

    # Install Icon
    install(
        FILES ${COLOBOT_ICON_FILE}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/
        )

    # Create manpage from pod-formatted file
    find_program(POD2MAN pod2man)
    if(NOT POD2MAN)
        message(WARNING "pod2man not found; manpage will not be generated")
    endif()

    if(POD2MAN)
        set(COLOBOT_MANPAGE_SECTION 6)

        macro(podman)
            cmake_parse_arguments(PM "" "PODFILE;LOCALE;" "" ${ARGN})
            if(PM_LOCALE)
                # This copes with the fact that english has no "/LANG" in the paths and filenames.
                set(SLASHLOCALE /${PM_LOCALE})
            endif()
            file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE})
            # Depending on coverage, colobot.pod (PM_PODFILE) might not exist, bear with its inexistance
            add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${PM_PODFILE}
                       COMMAND ${POD2MAN} ARGS --section=${COLOBOT_MANPAGE_SECTION}
                                          --center="Colobot" --stderr --utf8
                                          --release="${COLOBOT_VERSION_FULL}"
                                          ${CMAKE_CURRENT_SOURCE_DIR}/${PM_PODFILE}
                                          ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
                                          || rm -f ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
                        COMMENT "Create ${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION} manpage"
                      )
            add_custom_target(man${PM_LOCALE} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION})

            install(
                FILES ${CMAKE_CURRENT_BINARY_DIR}${SLASHLOCALE}/colobot.${COLOBOT_MANPAGE_SECTION}
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man${SLASHLOCALE}/man${COLOBOT_MANPAGE_SECTION}/
                OPTIONAL
            )

            add_dependencies(man man${PM_LOCALE})
        endmacro()

        # Create the english manpage
        podman(PODFILE colobot.pod)

        # Translate translatable material
        find_program(PO4A po4a)
        if(NOT PO4A)
            message(WARNING "po4a not found; desktop and manpage files will not be translated")
        endif()

        if(PO4A)
            # Translate the manpage to other languages
            file(GLOB LINGUAS_PO RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/po/ ${CMAKE_CURRENT_SOURCE_DIR}/po/*.po)
            string(REGEX REPLACE ".po" ";" LINGUAS ${LINGUAS_PO})

            set(PO4A_OUTPUTS)
            foreach(LOCALE ${LINGUAS})
                list(APPEND PO4A_OUTPUTS
                        ${CMAKE_CURRENT_SOURCE_DIR}/lang/${LOCALE}/colobot.pod
                        ${CMAKE_CURRENT_SOURCE_DIR}/lang/${LOCALE}/colobot.ini
                    )
            endforeach()
            add_custom_command(
                OUTPUT ${PO4A_OUTPUTS}
                COMMAND ${PO4A} po4a.cfg
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
            add_custom_target(desktop_po4a DEPENDS ${PO4A_OUTPUTS})
            add_dependencies(man desktop_po4a)
            add_dependencies(desktopfile desktop_po4a)

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

if(WIN32)
    set(COLOBOT_VERSION_4COMMAS "${CMAKE_PROJECT_VERSION_MAJOR},${CMAKE_PROJECT_VERSION_MINOR},${CMAKE_PROJECT_VERSION_PATCH},${CMAKE_PROJECT_VERSION_TWEAK}")
    configure_file(colobot.rc.cmake ${CMAKE_CURRENT_BINARY_DIR}/colobot.rc)
endif()


##
# Packaging
##
set(CPACK_BUNDLE_NAME "Colobot: Gold Edition")
set(CPACK_PACKAGE_FILE_NAME "colobot-${COLOBOT_VERSION_FULL}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Colobot: Gold Edition")
set(CPACK_PACKAGE_VENDOR "TerranovaTeam")

set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})

if(WIN32)
    set(CPACK_STRIP_FILES TRUE)

    # Don't version the install directory, and allow overwriting
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "Colobot")
    set(CPACK_NSIS_DEFINES "SetOverwrite on\nBrandingText \\\"Colobot: Gold Edition (${COLOBOT_VERSION_FULL})\\\"")
    # Install the executable directly in Program Files/Colobot/
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set(CPACK_PACKAGE_EXECUTABLES "colobot" "Colobot: Gold Edition")

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
    set(CPACK_BUNDLE_NAME "Colobot")
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
