if(COLOBOT_CORE_STATIC)
    add_library(colobot_core STATIC)
else()
    add_library(colobot_core SHARED)
    install(TARGETS colobot_core
            LIBRARY DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            ARCHIVE DESTINATION ${COLOBOT_INSTALL_LIB_DIR}
            RUNTIME DESTINATION ${COLOBOT_INSTALL_BIN_DIR})
endif()

target_sources(colobot_core PRIVATE
    stringutils.cpp
)

target_include_directories(colobot_core
    PUBLIC include
    PRIVATE ${ICU_INCLUDE_DIR}
)
target_link_libraries(colobot_core PRIVATE ${ICU_UC_LIBRARY} ${ICU_DT_LIBRARY})
