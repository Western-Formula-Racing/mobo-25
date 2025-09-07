# WiFi CAN Data Upload Feature

This feature enables the ESP32 to automatically connect to WiFi and upload CAN bus data to a remote server when the vehicle is in charging state. It supports both university eduroam (WPA2 Enterprise) and regular WiFi networks with automatic fallback.

## Configuration

Before building the project, you need to configure your WiFi credentials in the `include/config.h` file:

```cpp
//WiFi Configuration (Update these with your WiFi credentials)
// Primary WiFi (eduroam - WPA2 Enterprise)
#define WIFI_SSID_PRIMARY      "eduroam"
#define WIFI_USERNAME          "your.email@university.edu"
#define WIFI_PASSWORD_PRIMARY  "your_password"

// Fallback WiFi (Normal WPA2 Personal)
#define WIFI_SSID_FALLBACK     "YOUR_FALLBACK_SSID"
#define WIFI_PASSWORD_FALLBACK "YOUR_FALLBACK_PASSWORD"

#define WIFI_UPLOAD_URL "http://3.98.181.12:8085/can"
#define WIFI_CONNECTION_TIMEOUT_MS 20000  // 20 seconds timeout

// Time synchronization settings
#define TIMEZONE "UTC"  // Change to your timezone e.g., "EST5EDT", "PST8PDT", "CET-1CEST"
```

Replace:
- `your.email@university.edu` with your university email/username
- `your_password` with your university password
- `YOUR_FALLBACK_SSID` with a backup WiFi network name
- `YOUR_FALLBACK_PASSWORD` with the backup WiFi password
- `TIMEZONE` with your local timezone (default is UTC)
- The upload URL if you're using a different server

## How It Works

1. **Automatic Activation**: WiFi is automatically enabled when the vehicle enters the `CHARGING` state
2. **Smart Connection**: 
   - First attempts to connect to eduroam (20-second timeout)
   - If eduroam fails, automatically tries the fallback WiFi network
   - Logs connection attempts and results for troubleshooting
3. **Data Buffering**: All CAN messages (both received and transmitted) are buffered in memory during charging
4. **Periodic Upload**: Every 5 seconds, buffered CAN data is uploaded to the configured server via HTTP POST
5. **Automatic Cleanup**: WiFi is disabled when exiting the charging state to save power

## Data Format

The CAN data is uploaded in JSON format matching the protocol you specified:

```json
{
  "messages": [
    { "id": "171",  "data": [0, 0, 0, 0, 0, 0, 0, 0], "timestamp": 1648123456.789 },
    { "id": "172",  "data": [0, 0, 0, 0, 254, 219, 0, 0], "timestamp": 1648123456.789 }
  ]
}
```

Where:
- `id`: CAN message ID as a string
- `data`: 8-byte array of CAN data
- `timestamp`: **Unix epoch timestamp** in seconds with millisecond precision (synchronized via NTP when connected)

## Timestamp Handling

The system provides **real Unix epoch timestamps** with millisecond precision:

1. **Time Synchronization**: When WiFi connects, the system automatically synchronizes time using NTP servers
2. **Real Timestamps**: Once synchronized, all CAN messages get real Unix epoch timestamps
3. **Fallback**: If time sync fails, relative timestamps (since boot) are used with a warning
4. **Precision**: Timestamps include millisecond precision for accurate timing analysis

**Example timestamp**: `1648123456.789` = March 24, 2022 15:44:16.789 UTC

## Features

- **University-Ready**: Full support for eduroam (WPA2 Enterprise) authentication
- **Smart Fallback**: Automatic fallback to secondary WiFi if eduroam fails
- **Configurable Timeout**: 20-second timeout for each connection attempt
- **Low Power**: WiFi only activates during charging to minimize power consumption
- **Robust Buffering**: Messages are buffered in a circular buffer that prevents memory overflow
- **Error Handling**: Network failures are handled gracefully with automatic retry
- **Non-blocking**: CAN bus operation continues normally during WiFi uploads
- **Detailed Logging**: Comprehensive logs for connection troubleshooting

## Connection Priority

1. **Primary**: Attempts eduroam connection (WPA2 Enterprise)
2. **Fallback**: If eduroam fails within 20 seconds, tries normal WiFi
3. **Timeout**: Each connection attempt has a 20-second timeout
4. **Retry**: Failed connections are retried automatically

## Buffer Configuration

The system uses a circular buffer of 100 CAN messages by default. This can be modified in `wifi_manager.cpp`:

```cpp
#define CAN_BUFFER_SIZE 100
```

## Troubleshooting

1. **Eduroam Connection Issues**:
   - Verify your university username/email format
   - Ensure password is correct
   - Check if your university requires certificate validation (may need additional setup)
   - Monitor logs for specific authentication errors

2. **Fallback WiFi Issues**:
   - Verify SSID and password in config.h
   - Check WiFi signal strength at charging location
   - Ensure the fallback network uses WPA2-PSK security

3. **General Connection Issues**:
   - Check 20-second timeout is sufficient for your network
   - Monitor logs for detailed connection attempts
   - Verify both networks are available at charging location

4. **Upload Failures**:
   - Verify server URL and availability
   - Check network connectivity after successful WiFi connection
   - Monitor HTTP response codes in logs

5. **Memory Issues**:
   - Reduce CAN_BUFFER_SIZE if experiencing memory problems
   - Monitor heap usage during operation

## Log Output

The system provides detailed logging for both connection types:
- **Primary Connection**: "Attempting to connect to eduroam..."
- **Fallback Connection**: "Primary WiFi failed, trying fallback..."
- **Success Messages**: IP address assignment and connection confirmation
- **Failure Messages**: Specific timeout and authentication errors
- **Upload Status**: HTTP response codes and data transfer results

Monitor the ESP32 logs to verify proper operation and troubleshoot connection issues.
