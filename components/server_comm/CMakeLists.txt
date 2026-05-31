idf_component_register(
    SRCS "server_comm.c"
    INCLUDE_DIRS "include"
    REQUIRES
        http_client
        wifi
        nvs_flash
        esp_app_format
        esp_https_ota
        bootloader_support
        esp_partition
        driver
        freertos
)