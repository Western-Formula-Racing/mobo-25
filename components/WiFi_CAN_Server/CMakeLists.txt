idf_component_register(SRCS "WiFi_CAN_Server.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_server esp_wifi nvs_flash CAN)