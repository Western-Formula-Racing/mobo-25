#include "wifi_manager.h"

static const char *TAG = "WiFi_Manager";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static bool wifi_connected = false;
static bool trying_primary = true;
static bool wifi_initialized = false;
static bool time_synchronized = false;

// Buffer for CAN messages
#define CAN_BUFFER_SIZE 100
static can_message_t can_buffer[CAN_BUFFER_SIZE];
static size_t buffer_index = 0;
static bool upload_in_progress = false;

static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP (attempt %d/%d)", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGW(TAG, "Failed to connect after %d attempts", WIFI_MAXIMUM_RETRY);
        }
        wifi_connected = false;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected successfully! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        wifi_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        
        // Initialize SNTP when we get connected
        if (!time_synchronized) {
            initialize_sntp();
        }
    }
}

void wifi_init_sta(void)
{
    if (wifi_initialized) {
        return;
    }

    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi initialized");
}

bool try_connect_primary_wifi(void)
{
    ESP_LOGI(TAG, "Attempting to connect to eduroam...");
    
    // Configure for WPA2 Enterprise (eduroam)
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD_PRIMARY, strlen(WIFI_PASSWORD_PRIMARY)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_enable());

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID_PRIMARY,
            .password = "",  // Password is set via WPA2 enterprise
            .threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Reset retry counter
    s_retry_num = 0;
    
    // Wait for connection with timeout
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,  // Clear bits after waiting
            pdFALSE,
            pdMS_TO_TICKS(WIFI_PRIMARY_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Successfully connected to eduroam");
        return true;
    } else {
        ESP_LOGW(TAG, "Failed to connect to eduroam within timeout");
        esp_wifi_stop();
        esp_wifi_sta_wpa2_ent_disable();
        return false;
    }
}

bool try_connect_fallback_wifi(void)
{
    ESP_LOGI(TAG, "Attempting to connect to fallback WiFi: %s", WIFI_SSID_FALLBACK);
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID_FALLBACK,
            .password = WIFI_PASSWORD_FALLBACK,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Reset retry counter
    s_retry_num = 0;
    
    // Wait for connection with timeout
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,  // Clear bits after waiting
            pdFALSE,
            pdMS_TO_TICKS(WIFI_PRIMARY_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Successfully connected to fallback WiFi: %s", WIFI_SSID_FALLBACK);
        return true;
    } else {
        ESP_LOGW(TAG, "Failed to connect to fallback WiFi");
        esp_wifi_stop();
        return false;
    }
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time synchronized successfully");
    time_synchronized = true;
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP for time synchronization...");
    
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.nist.gov");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    // Set timezone from config, default to UTC if not defined
    #ifndef TIMEZONE
    #define TIMEZONE "UTC"
    #endif
    
    const char* tz = TIMEZONE;
    if (strlen(tz) == 0) {
        tz = "UTC";  // Fallback to UTC if empty
    }
    
    setenv("TZ", tz, 1);
    tzset();
    
    esp_sntp_init();
    
    ESP_LOGI(TAG, "SNTP initialized with timezone %s (UTC default), waiting for time sync...", tz);
}

double get_epoch_timestamp_with_ms(void)
{
    if (!time_synchronized) {
        // If time not synchronized, fall back to relative time with a warning
        ESP_LOGW(TAG, "Time not synchronized, using relative timestamp");
        return (double)pdTICKS_TO_MS(xTaskGetTickCount()) / 1000.0;
    }
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    // Convert to double with millisecond precision
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

void wifi_start(void)
{
    if (wifi_connected) {
        ESP_LOGI(TAG, "WiFi already connected");
        return;
    }

    ESP_LOGI(TAG, "Starting WiFi connection process...");
    
    // First try eduroam (primary)
    if (try_connect_primary_wifi()) {
        trying_primary = true;
        return;
    }
    
    // If eduroam fails, try fallback WiFi
    ESP_LOGI(TAG, "Primary WiFi failed, trying fallback...");
    if (try_connect_fallback_wifi()) {
        trying_primary = false;
        return;
    }
    
    // Both failed
    ESP_LOGE(TAG, "Failed to connect to both primary and fallback WiFi networks");
}

void wifi_stop(void)
{
    if (wifi_connected || wifi_initialized) {
        ESP_LOGI(TAG, "Stopping WiFi...");
        if (wifi_connected) {
            esp_wifi_disconnect();
        }
        esp_wifi_stop();
        if (trying_primary) {
            esp_wifi_sta_wpa2_ent_disable();
        }
        
        // Stop SNTP if it was running
        if (time_synchronized) {
            esp_sntp_stop();
            time_synchronized = false;
            ESP_LOGI(TAG, "SNTP stopped");
        }
        
        wifi_connected = false;
        ESP_LOGI(TAG, "WiFi stopped");
    }
}

bool is_wifi_connected(void)
{
    return wifi_connected;
}

esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        case HTTP_EVENT_REDIRECT:
            break;
    }
    return ESP_OK;
}

esp_err_t upload_can_data(can_message_t* messages, size_t count)
{
    if (!wifi_connected || count == 0) {
        return ESP_ERR_INVALID_STATE;
    }

    // Create JSON payload
    cJSON *json = cJSON_CreateObject();
    cJSON *messages_array = cJSON_CreateArray();
    
    for (size_t i = 0; i < count; i++) {
        cJSON *message = cJSON_CreateObject();
        
        // Convert ID to string
        char id_str[12];
        snprintf(id_str, sizeof(id_str), "%lu", (unsigned long)messages[i].id);
        cJSON_AddStringToObject(message, "id", id_str);
        
        // Add data array
        cJSON *data_array = cJSON_CreateArray();
        for (int j = 0; j < 8; j++) {
            cJSON_AddItemToArray(data_array, cJSON_CreateNumber(messages[i].data[j]));
        }
        cJSON_AddItemToObject(message, "data", data_array);
        
        // Add timestamp
        cJSON_AddNumberToObject(message, "timestamp", messages[i].timestamp);
        
        cJSON_AddItemToArray(messages_array, message);
    }
    
    cJSON_AddItemToObject(json, "messages", messages_array);
    
    char *json_string = cJSON_Print(json);
    if (json_string == NULL) {
        cJSON_Delete(json);
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "Uploading %zu CAN messages", count);

    esp_http_client_config_t config = {
        .url = WIFI_UPLOAD_URL,
        .event_handler = http_event_handler,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_data(client, json_string, strlen(json_string));

    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP POST Status = %d", status_code);
        if (status_code >= 200 && status_code < 300) {
            ESP_LOGI(TAG, "CAN data uploaded successfully");
        } else {
            ESP_LOGW(TAG, "HTTP request failed with status %d", status_code);
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    free(json_string);
    cJSON_Delete(json);
    
    return err;
}

void add_can_message_to_buffer(uint32_t id, uint8_t* data, double timestamp)
{
    if (buffer_index < CAN_BUFFER_SIZE) {
        can_buffer[buffer_index].id = id;
        memcpy(can_buffer[buffer_index].data, data, 8);
        can_buffer[buffer_index].timestamp = timestamp;
        buffer_index++;
    } else {
        // Buffer full, shift messages and add new one at the end
        memmove(&can_buffer[0], &can_buffer[1], (CAN_BUFFER_SIZE - 1) * sizeof(can_message_t));
        can_buffer[CAN_BUFFER_SIZE - 1].id = id;
        memcpy(can_buffer[CAN_BUFFER_SIZE - 1].data, data, 8);
        can_buffer[CAN_BUFFER_SIZE - 1].timestamp = timestamp;
    }
}

void wifi_upload_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5000); // Upload every 5 seconds
    
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        if (wifi_connected && buffer_index > 0 && !upload_in_progress) {
            upload_in_progress = true;
            
            // Copy buffer to avoid blocking CAN reception
            size_t messages_to_upload = buffer_index;
            can_message_t upload_buffer[CAN_BUFFER_SIZE];
            memcpy(upload_buffer, can_buffer, messages_to_upload * sizeof(can_message_t));
            
            // Reset buffer
            buffer_index = 0;
            
            // Upload data
            esp_err_t result = upload_can_data(upload_buffer, messages_to_upload);
            if (result != ESP_OK) {
                ESP_LOGW(TAG, "Failed to upload CAN data, will retry next cycle");
            }
            
            upload_in_progress = false;
        }
    }
}
