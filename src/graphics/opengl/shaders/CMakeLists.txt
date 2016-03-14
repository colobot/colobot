file(GLOB shaders *.glsl)
install(FILES ${shaders} DESTINATION ${COLOBOT_INSTALL_DATA_DIR}/shaders)
install(DIRECTORY gl21 DESTINATION ${COLOBOT_INSTALL_DATA_DIR}/shaders)
