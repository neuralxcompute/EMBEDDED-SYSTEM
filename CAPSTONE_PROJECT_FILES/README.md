# Industrial IoT Automation System

## Project Title
**Industrial IoT Automation System Using ESP32 and HiveMQ Cloud for Remote Monitoring and Control**

## Project Description
An ESP32-based Industrial Internet of Things prototype that combines environmental sensing, local automatic fan control, OLED indication, Wi-Fi connectivity and MQTT communication through HiveMQ Cloud.

## Problem Statement
The project provides low-cost remote visibility of environmental conditions while retaining local automatic control so that basic fan actuation does not depend entirely on cloud availability.

## Objectives
1. Acquire temperature and humidity using DHT11.
2. Acquire the MQ-2 analog signal as a raw ADC value.
3. Display temperature, humidity, gas value, fan state and operating mode on the OLED.
4. Implement automatic fan control when temperature > 30 °C OR MQ-2 ADC > 700.
5. Connect ESP32 to Wi-Fi and HiveMQ Cloud using MQTT.
6. Publish telemetry containing temperature, humidity, gas, fan and mode.
7. Receive AUTO, ON and OFF commands through MQTT.
8. Implement MQTT reconnection handling.
9. Verify the integrated prototype using requirement-linked tests.

## Hardware Requirements
- ESP32 development board
- DHT11 temperature/humidity sensor
- MQ-2 sensor
- 0.96-inch SSD1306 OLED, 128 × 64
- Relay module
- Demonstration fan/load
- Breadboard and jumper wires
- USB programming/debugging cable
- Regulated power arrangement appropriate to the actual prototype

## Software Requirements
- Arduino IDE
- ESP32 Arduino Core
- DHT library
- PubSubClient
- Adafruit GFX Library
- Adafruit SSD1306 Library

See `software/libraries.txt` and `software/setup_instructions.md`.

## Circuit / System Diagram
See:
- `circuit_diagram/system_architecture.png`
- `circuit_diagram/system_block_diagram.png`
- `circuit_diagram/circuit_interconnection.png`
- `circuit_diagram/software_flowchart.png`

## GPIO Mapping
| ESP32 Pin | Device | Function |
|---|---|---|
| GPIO 4 | DHT11 DATA | Temperature/humidity |
| GPIO 34 | MQ-2 AO | Raw ADC gas/smoke-related signal |
| GPIO 26 | Relay IN | Fan control |
| GPIO 21 | OLED SDA | I2C data |
| GPIO 22 | OLED SCL | I2C clock |

## MQTT Topics
- `industrial/site1/telemetry` — ESP32 → broker
- `industrial/site1/command` — Client → ESP32
- `industrial/site1/status` — ESP32 → broker

Commands:
- `AUTO`
- `ON`
- `OFF`

## Installation / Setup
1. Install Arduino IDE.
2. Install ESP32 board support.
3. Install the libraries listed in `software/libraries.txt`.
4. Open `source_code/industrial_iot_clean.ino`.
5. Enter the actual Wi-Fi SSID/password and valid HiveMQ credentials locally.
6. Do not commit credentials to GitHub.
7. Select the correct ESP32 board and COM port.
8. Compile and upload the sketch.
9. Open Serial Monitor at 115200 baud.

## How to Run
1. Connect the hardware according to the circuit diagrams.
2. Power the prototype safely.
3. Upload the firmware.
4. Open Serial Monitor at 115200 baud.
5. Confirm Wi-Fi connection.
6. Confirm HiveMQ connection and `ESP32 ONLINE` status.
7. Observe the OLED.
8. Observe MQTT telemetry.
9. Send `AUTO`, `ON`, or `OFF` to `industrial/site1/command`.

## How to Operate
### AUTO
Fan turns ON when:
- Temperature > 30 °C, OR
- MQ-2 ADC > 700

Otherwise the fan remains OFF.

### Manual ON
Send `ON` to the command topic. The fan turns ON and manual mode is selected.

### Manual OFF
Send `OFF` to the command topic. The fan turns OFF and manual mode is selected.

### Return to AUTO
Send `AUTO` to the command topic. The threshold rule is immediately evaluated.

## Testing Procedure
Run the tests listed in `test_results/test_plan.csv`.
The verification results are in:
- `test_results/observed_results.csv`
- `test_results/verification_validation.csv`

## Experimental Data
The currently documented dataset contains one traceable OLED operating snapshot:
- Temperature: 27.2 °C
- Humidity: 45.0 %
- MQ-2 ADC: 105
- Fan: OFF
- Mode: AUTO

The project record does not contain enough repeated observations to calculate meaningful sensor accuracy, standard deviation, MQTT latency distribution, packet loss or long-duration reliability. These are therefore not fabricated.

## Results
Functional evidence supports:
- ESP32 startup
- Wi-Fi connection
- DHT11 acquisition
- MQ-2 acquisition
- OLED display
- AUTO fan control
- Manual ON/OFF commands
- MQTT telemetry
- Online status

MQTT reconnection is implemented but lacks a repeated recovery dataset.

## Team Members
- Paul David — 192572144

> Add the remaining officially approved team members if this is being submitted as a team repository. Do not invent roles or contribution percentages.

## References
Use the references listed in the final capstone report and the documentation for the Arduino/ESP32, MQTT and display libraries used by the implementation.

## Repository Structure
```text
Industrial-IoT-Automation-System/
├── README.md
├── source_code/
│   └── industrial_iot_clean.ino
├── circuit_diagram/
├── hardware/
├── software/
├── dataset/
├── test_results/
├── documentation/
└── images/
```

## Security Warning
The prototype source uses `WiFiClientSecure` with `setInsecure()`. This means TLS transport is used but server certificate verification is disabled. Replace this with proper certificate validation before deployment.

Do not use this educational prototype as a certified gas alarm, emergency ventilation controller, or production industrial safety system.
