
function(DAMIAO_install_elf TARGET)
    message("Add Algorithm: " ${TARGET})
    install(
        TARGETS ${TARGET} RUNTIME
        DESTINATION ${JLINK_DEIVCE_DIR}/DAMIAO
    )
endfunction(DAMIAO_install_elf)

if (${BOARD_NAME} STREQUAL "ALL")
    file(GLOB BOARD_DIRS LIST_DIRECTORIES true board_*)
else()
    list(TRANSFORM BOARD_NAME PREPEND "board_" OUTPUT_VARIABLE BOARD_DIRS)
endif()

foreach(BOARD_DIR IN LISTS BOARD_DIRS)    
    get_filename_component(BOARD_DIR_NAME ${BOARD_DIR} NAME)
    add_subdirectory(${BOARD_DIR})
endforeach()

install(FILES JLinkDevices.xml
    DESTINATION ${JLINK_DEIVCE_DIR}/DAMIAO
)
