#ifndef TEST_MODE_H
#define TEST_MODE_H

// Uncomment this line to enable test mode
#define ENABLE_TEST_MODE 1

#ifdef ENABLE_TEST_MODE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_manager.h"

// Test mode functions
void run_wifi_tests(void);
void simulate_can_messages(void);
void test_state_transitions(void);
void test_error_conditions(void);

// Mock functions for testing
typedef enum {
    TEST_IDLE,
    TEST_CHARGING,
    TEST_ACTIVE,
    TEST_FAULT
} test_state_t;

extern test_state_t test_current_state;

// Override real functions with test versions
#define getStatus() test_current_state
#define getPackCurrent() 25.5
#define getMaxVoltage() 4.0
#define getMinVoltage() 3.8

#endif // ENABLE_TEST_MODE

#endif // TEST_MODE_H
