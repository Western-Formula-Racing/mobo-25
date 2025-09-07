#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "config.h"
#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>

#define WIFI_MAXIMUM_RETRY  3
#define WIFI_PRIMARY_TIMEOUT_MS WIFI_CONNECTION_TIMEOUT_MS

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

struct can_message_t {
    uint32_t id;
    uint8_t data[8];
    double timestamp;
};

void wifi_init_sta(void);
void wifi_start(void);
void wifi_stop(void);
bool is_wifi_connected(void);
esp_err_t upload_can_data(can_message_t* messages, size_t count);
void wifi_upload_task(void *pvParameters);
void add_can_message_to_buffer(uint32_t id, uint8_t* data, double timestamp);
bool try_connect_primary_wifi(void);
bool try_connect_fallback_wifi(void);
void initialize_sntp(void);
double get_epoch_timestamp_with_ms(void);

#endif
