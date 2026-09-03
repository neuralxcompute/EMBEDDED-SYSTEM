# Software Setup

1. Install Arduino IDE.
2. Install ESP32 board support.
3. Install:
   - DHT sensor library
   - PubSubClient
   - Adafruit GFX Library
   - Adafruit SSD1306 Library
4. Open `source_code/industrial_iot_clean.ino`.
5. Select the correct ESP32 board.
6. Select the correct COM port.
7. Replace the credential placeholders locally.
8. Verify/compile.
9. Upload to ESP32.
10. Open Serial Monitor at 115200 baud.

## Credential rule
Never commit Wi-Fi passwords, HiveMQ passwords, API keys or tokens.
