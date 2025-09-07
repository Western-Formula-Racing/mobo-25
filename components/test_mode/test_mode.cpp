#include "test_mode.h"

#ifdef ENABLE_TEST_MODE

static const char *TAG = "TEST_MODE";
test_state_t test_current_state = TEST_IDLE;

void run_wifi_tests(void)
{
    ESP_LOGI(TAG, "=== Starting WiFi Tests ===");
    
    // Test WiFi initialization
    wifi_init_sta();
    
    // Test WiFi connection
    wifi_start();
    
    // Wait for connection
    vTaskDelay(pdMS_TO_TICKS(30000)); // 30 seconds
    
    if (is_wifi_connected()) {
        ESP_LOGI(TAG, "✅ WiFi connection test PASSED");
        
        // Test time synchronization
        vTaskDelay(pdMS_TO_TICKS(10000)); // Wait for time sync
        
        double timestamp = get_epoch_timestamp_with_ms();
        ESP_LOGI(TAG, "Current timestamp: %.3f", timestamp);
        
        if (timestamp > 1600000000) { // After 2020
            ESP_LOGI(TAG, "✅ Time synchronization test PASSED");
        } else {
            ESP_LOGW(TAG, "⚠️ Time synchronization may have failed");
        }
    } else {
        ESP_LOGE(TAG, "❌ WiFi connection test FAILED");
    }
}

void simulate_can_messages(void)
{
    ESP_LOGI(TAG, "=== Starting CAN Message Simulation ===");
    
    // Simulate entering charging state
    test_current_state = TEST_CHARGING;
    ESP_LOGI(TAG, "Simulating CHARGING state");
    
    // Generate test CAN messages similar to real BMS data
    uint8_t test_data1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t test_data2[] = {0x00, 0x00, 0x00, 0x00, 0xFE, 0xDB, 0x00, 0x00};
    uint8_t test_data3[] = {0x70, 0x06, 0xBA, 0xF9, 0x00, 0x00, 0x00, 0x00};
    
    // Buffer test messages
    for (int i = 0; i < 10; i++) {
        buffer_can_message_for_upload(0x171, test_data1);
        buffer_can_message_for_upload(0x172, test_data2); 
        buffer_can_message_for_upload(0x177, test_data3);
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms between messages
    }
    
    ESP_LOGI(TAG, "Generated 30 test CAN messages");
    
    // Wait for upload
    vTaskDelay(pdMS_TO_TICKS(15000)); // 15 seconds
    
    // Exit charging state
    test_current_state = TEST_ACTIVE;
    ESP_LOGI(TAG, "Exited CHARGING state");
}

void test_state_transitions(void)
{
    ESP_LOGI(TAG, "=== Starting State Transition Tests ===");
    
    // Test state sequence: IDLE -> CHARGING -> ACTIVE
    test_current_state = TEST_IDLE;
    ESP_LOGI(TAG, "State: IDLE");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    test_current_state = TEST_CHARGING;
    ESP_LOGI(TAG, "State: CHARGING (WiFi should start)");
    vTaskDelay(pdMS_TO_TICKS(10000)); // 10 seconds in charging
    
    test_current_state = TEST_ACTIVE;
    ESP_LOGI(TAG, "State: ACTIVE (WiFi should stop)");
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test fault condition during charging
    test_current_state = TEST_CHARGING;
    ESP_LOGI(TAG, "State: CHARGING again");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    test_current_state = TEST_FAULT;
    ESP_LOGI(TAG, "State: FAULT (WiFi should stop immediately)");
    vTaskDelay(pdMS_TO_TICKS(2000));
}

void test_error_conditions(void)
{
    ESP_LOGI(TAG, "=== Starting Error Condition Tests ===");
    
    // Test with WiFi disconnected
    wifi_stop();
    test_current_state = TEST_CHARGING;
    
    // Try to buffer messages without WiFi
    uint8_t test_data[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
    buffer_can_message_for_upload(0x123, test_data);
    
    ESP_LOGI(TAG, "Tested message buffering without WiFi");
    
    // Test WiFi reconnection
    wifi_start();
    vTaskDelay(pdMS_TO_TICKS(30000)); // Wait for reconnection
    
    if (is_wifi_connected()) {
        ESP_LOGI(TAG, "✅ WiFi reconnection test PASSED");
    } else {
        ESP_LOGE(TAG, "❌ WiFi reconnection test FAILED");
    }
}

void run_all_tests(void)
{
    ESP_LOGI(TAG, "🧪 Starting comprehensive WiFi/CAN tests...");
    
    run_wifi_tests();
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    simulate_can_messages(); 
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    test_state_transitions();
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    test_error_conditions();
    
    ESP_LOGI(TAG, "🎉 All tests completed!");
}

#endif // ENABLE_TEST_MODE
