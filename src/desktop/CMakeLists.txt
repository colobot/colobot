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

find_program(PO4A po4a)

if(PO4A)
	add_custom_target(desktop_po4a
		COMMAND ${PO4A} po4a.cfg
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	add_dependencies(desktopfile desktop_po4a)
endif()
