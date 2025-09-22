#include "WiFi_CAN_Server.h"
#include "CAN.h"
#include <string.h>

static const char *TAG = "WiFi_CAN_Server";

static const httpd_uri_t start_balance_uri = {
    .uri       = "/start_balance",
    .method    = HTTP_GET,
    .handler   = WiFi_CAN_Server::start_balance_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t end_balance_uri = {
    .uri       = "/end_balance",
    .method    = HTTP_GET,
    .handler   = WiFi_CAN_Server::end_balance_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = WiFi_CAN_Server::root_handler,
    .user_ctx  = NULL
};

void WiFi_CAN_Server::init(const char* ssid, const char* password) {
    ESP_LOGI(TAG, "Initializing WiFi CAN Server...");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.ap.ssid, ssid);
    strcpy((char*)wifi_config.ap.password, password);
    wifi_config.ap.ssid_len = strlen(ssid);
    wifi_config.ap.channel = 1;
    wifi_config.ap.max_connection = 4;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID: %s", ssid);
}

void WiFi_CAN_Server::startServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    ESP_LOGI(TAG, "Starting HTTP server...");
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &start_balance_uri);
        httpd_register_uri_handler(server, &end_balance_uri);
        httpd_register_uri_handler(server, &root_uri);
        ESP_LOGI(TAG, "HTTP server started");
    } else {
        ESP_LOGE(TAG, "Failed to start HTTP server");
    }
}

void WiFi_CAN_Server::stopServer() {
    if (server) {
        httpd_stop(server);
        server = NULL;
        ESP_LOGI(TAG, "HTTP server stopped");
    }
}

esp_err_t WiFi_CAN_Server::start_balance_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received start balance request");
    startBalanceMessage();
    const char* resp = "Start balancing message sent";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t WiFi_CAN_Server::end_balance_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received end balance request");
    balanceMessage();
    const char* resp = "End balancing message sent";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}

esp_err_t WiFi_CAN_Server::root_handler(httpd_req_t *req) {
    const char* resp = "<html><body><h1>Motherboard CAN Control</h1><p><a href='/start_balance'>Start Balancing</a></p><p><a href='/end_balance'>End Balancing</a></p></body></html>";
    httpd_resp_send(req, resp, strlen(resp));
    return ESP_OK;
}