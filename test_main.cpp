#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "test_mode.h"

extern "C" void app_main(void)
{
    static const char* TAG = "TEST_MAIN";
    
    ESP_LOGI(TAG, "🧪 ESP32 WiFi/CAN Test Mode Starting...");
    
    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
#ifdef ENABLE_TEST_MODE
    ESP_LOGI(TAG, "✅ Test mode enabled - running safe tests");
    
    // Wait a bit for system to stabilize
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Run all tests
    run_all_tests();
    
    // Keep running for monitoring
    while(1) {
        ESP_LOGI(TAG, "Test mode running... Monitor output above");
        vTaskDelay(pdMS_TO_TICKS(30000)); // Log every 30 seconds
    }
    
#else
    ESP_LOGE(TAG, "❌ TEST_MODE not enabled! Enable in test_mode.h");
    ESP_LOGE(TAG, "This prevents accidental flashing to production BMS");
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
#endif
}
