idf_component_register(SRCS "wifi_manager.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES esp_wifi esp_netif esp_http_client json nvs_flash wpa_supplicant lwip)
