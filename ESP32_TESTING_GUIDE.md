# ESP32 Development Board Testing Guide

## 🔧 Hardware Setup
1. Use any ESP32 development board (ESP32-DevKitC, ESP32-WROOM, etc.)
2. Optional: Connect CAN transceiver module (MCP2515 or SN65HVD230) for CAN testing
3. **NO high-voltage connections needed - completely safe**

## 🛡️ Safety Features
- Test mode prevents accidental production flashing
- All testing done on isolated development board
- No BMS/accumulator hardware required
- Code validation before high-risk deployment

## 📋 Step-by-Step Testing Process

### Step 1: Prepare Test Environment

1. **Set up test server** (on your computer):
   ```bash
   cd /path/to/mobo-25
   pip install flask
   python test_server.py
   ```

2. **Update test configuration** in `components/test_mode/test_mode.h`:
   ```cpp
   #define ENABLE_TEST_MODE 1  // Enable this line
   ```

3. **Configure WiFi** in `include/config.h`:
   ```cpp
   #define WIFI_SSID_PRIMARY      "eduroam"
   #define WIFI_USERNAME          "your.actual@email.edu"  
   #define WIFI_PASSWORD_PRIMARY  "your_actual_password"
   #define WIFI_SSID_FALLBACK     "your_test_wifi"
   #define WIFI_PASSWORD_FALLBACK "test_password"
   #define WIFI_UPLOAD_URL "http://YOUR_COMPUTER_IP:8085/can"  // Replace with your IP
   ```

### Step 2: Build and Flash Test Version

1. **Replace main.cpp temporarily**:
   ```bash
   mv main/main.cpp main/main_production.cpp
   mv test_main.cpp main/main.cpp
   ```

2. **Build and flash**:
   ```bash
   idf.py build
   idf.py flash monitor
   ```

### Step 3: Run Tests and Monitor

Watch the serial output for test results:

- ✅ **WiFi Connection Tests**: Verifies eduroam and fallback
- ✅ **Time Sync Tests**: Confirms NTP synchronization  
- ✅ **CAN Message Tests**: Validates buffering and upload
- ✅ **State Transition Tests**: Tests WiFi start/stop logic
- ✅ **Error Handling Tests**: Verifies robustness

### Step 4: Validate Results

Check your test server output for:
- Proper JSON format
- Correct timestamps (Unix epoch)
- Expected CAN message IDs and data
- Appropriate upload frequency

### Step 5: Restore Production Code

```bash
mv main/main.cpp main/main_test.cpp
mv main/main_production.cpp main/main.cpp
```

## 🧪 What Gets Tested

### WiFi Functionality
- [x] eduroam (WPA2 Enterprise) connection
- [x] Fallback WiFi connection  
- [x] Connection timeout handling
- [x] Automatic reconnection
- [x] Proper cleanup on disconnect

### Time Synchronization
- [x] NTP server communication
- [x] UTC timezone configuration
- [x] Timestamp accuracy
- [x] Fallback to relative time

### CAN Data Handling  
- [x] Message buffering during charging state
- [x] JSON payload formation
- [x] HTTP POST with correct headers
- [x] Server communication
- [x] Buffer overflow protection

### State Machine Integration
- [x] WiFi activation in CHARGING state only
- [x] WiFi deactivation when exiting CHARGING
- [x] Proper cleanup on fault conditions
- [x] Memory leak prevention

## 📊 Expected Test Output

```
🧪 ESP32 WiFi/CAN Test Mode Starting...
✅ Test mode enabled - running safe tests
=== Starting WiFi Tests ===
I (12345) WiFi_Manager: Attempting to connect to eduroam...
I (15678) WiFi_Manager: Connected successfully! Got IP: 192.168.1.100
✅ WiFi connection test PASSED
I (18901) WiFi_Manager: Time synchronized successfully  
Current timestamp: 1725638400.123
✅ Time synchronization test PASSED
=== Starting CAN Message Simulation ===
Generated 30 test CAN messages
🎉 All tests completed!
```

## 🚨 Before Production Deployment

1. **All tests must pass** ✅
2. **Server receives proper data** ✅  
3. **No memory leaks observed** ✅
4. **WiFi connects reliably** ✅
5. **Timestamps are accurate** ✅
6. **Disable test mode** in `test_mode.h`
7. **Restore production main.cpp**
8. **Final build and flash to BMS**

This testing approach ensures your WiFi/CAN upload feature works perfectly before deploying to the high-risk accumulator environment! 🛡️
