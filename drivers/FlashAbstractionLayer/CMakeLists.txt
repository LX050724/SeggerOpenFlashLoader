add_library(FlashAbstractionLayer INTERFACE)
target_sources(FlashAbstractionLayer INTERFACE FlashAbstractionLayer.c)

target_include_directories(FlashAbstractionLayer INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

add_library(FAL_STM_HAL_SPI INTERFACE)
target_sources(FAL_STM_HAL_SPI INTERFACE FAL_STM_HAL_SPI.c)
target_link_libraries(FAL_STM_HAL_SPI INTERFACE FlashAbstractionLayer)

add_library(FAL_STM_HAL_QSPI INTERFACE)
target_sources(FAL_STM_HAL_QSPI INTERFACE FAL_STM_HAL_QSPI.c)
target_link_libraries(FAL_STM_HAL_QSPI INTERFACE FlashAbstractionLayer)

add_library(FAL_STM_HAL_OSPI INTERFACE)
target_sources(FAL_STM_HAL_OSPI INTERFACE FAL_STM_HAL_OSPI.c)
target_link_libraries(FAL_STM_HAL_OSPI INTERFACE FlashAbstractionLayer)

# add_library(FAL_STM_LL_SPI2S2_V1_0 INTERFACE)
# target_sources(FAL_STM_LL_SPI2S2_V1_0 INTERFACE FAL_STM_LL_SPI2S2_V1_0.c)
# target_link_libraries(FAL_STM_LL_SPI2S2_V1_0 INTERFACE FlashAbstractionLayer)
