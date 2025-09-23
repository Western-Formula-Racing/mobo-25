#ifndef WIFI_CAN_SERVER_H
#define WIFI_CAN_SERVER_H

#include "esp_http_server.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

class WiFi_CAN_Server {
public:
    void init(const char* ssid, const char* password);
    void startServer();
    void stopServer();

    static esp_err_t start_balance_handler(httpd_req_t *req);
    static esp_err_t end_balance_handler(httpd_req_t *req);
    static esp_err_t root_handler(httpd_req_t *req);

private:
    httpd_handle_t server = NULL;
};

#endif