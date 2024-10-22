
function(Alientek_install_elf TARGET)
    message("Add Algorithm: " ${TARGET})
    install(
        TARGETS ${TARGET} RUNTIME
        DESTINATION ${JLINK_DEIVCE_DIR}/Alientek
    )
endfunction(Alientek_install_elf)

if (${BOARD_NAME} STREQUAL "ALL")
    file(GLOB BOARD_DIRS LIST_DIRECTORIES true board_*)
else()
    set(BOARD_DIRS ${BOARD_NAME})
endif()

foreach(BOARD_DIR IN LISTS BOARD_DIRS)
    get_filename_component(BOARD_DIR_NAME ${BOARD_DIR} NAME)
    add_subdirectory(${BOARD_DIR})
endforeach()

install(FILES JLinkDevices.xml
    DESTINATION ${JLINK_DEIVCE_DIR}/Alientek
)

