idf_component_register(
    SRCS "ecomax.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        esp_driver_uart
        esp_timer
        helpers
)