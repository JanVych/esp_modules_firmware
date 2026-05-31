idf_component_register(
    SRCS "etatherm.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        esp_timer
        esp_driver_uart
)
