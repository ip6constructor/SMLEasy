if(IDF_TARGET STREQUAL "esp32s3" OR IDF_VERSION_MAJOR GREATER_EQUAL 6)
    set(SML_UART_DRIVER esp_driver_uart)
else()
    set(SML_UART_DRIVER driver)
endif()

idf_component_register(
    SRCS
        "src/sml_reader.cpp"
    INCLUDE_DIRS
        "include"
    REQUIRES
        ${SML_UART_DRIVER}
    PRIV_REQUIRES
        esp_timer
)
