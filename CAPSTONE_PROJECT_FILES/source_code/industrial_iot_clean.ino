/*
  Industrial IoT Automation System
  ESP32 + DHT11 + MQ-2 + SSD1306 OLED + Relay + HiveMQ Cloud

  Main/entry program: industrial_iot_clean.ino

  IMPORTANT:
  - Replace the Wi-Fi and HiveMQ password placeholders locally.
  - Never commit passwords, API keys, tokens, or private credentials.
  - The prototype currently uses setInsecure(), so TLS certificate
    verification is NOT enforced. Replace it with CA/server certificate
    validation before real deployment.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//
//
//
//
//
//
//
//
// =====================================================
// WIFI
// =====================================================

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// =====================================================
// HIVEMQ CLOUD
// =====================================================

const char* MQTT_HOST =
    "fd439c9af8fd4b9586de13f67ead11e1.s1.eu.hivemq.cloud";

const int MQTT_PORT = 8883;

const char* MQTT_USERNAME = "industrial_esp32";
const char* MQTT_PASSWORD = "YOUR_HIVEMQ_PASSWORD";

// =====================================================
// MQTT TOPICS
// =====================================================

const char* TOPIC_TELEMETRY = "industrial/site1/telemetry";
const char* TOPIC_COMMAND = "industrial/site1/command";
const char* TOPIC_STATUS = "industrial/site1/status";

// =====================================================
// PIN DEFINITIONS
// =====================================================

#define DHT_PIN 4
#define DHT_TYPE DHT11

#define MQ2_PIN 34

#define RELAY_PIN 26

#define OLED_SDA 21
#define OLED_SCL 22

// =====================================================
// OBJECTS
// =====================================================

DHT dht(DHT_PIN, DHT_TYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    -1
);

WiFiClientSecure secureClient;
PubSubClient mqttClient(secureClient);

// =====================================================
// VARIABLES
// =====================================================

float temperature = 0;
float humidity = 0;

int gasValue = 0;

bool fanState = false;

// AUTO mode initially
bool autoMode = true;

// =====================================================
// THRESHOLDS
// =====================================================

// Fan ON when temperature is ABOVE 30°C
const float FAN_ON_TEMP = 30.0;

// Fan ON when MQ-2 ADC reading is ABOVE 700
const int GAS_THRESHOLD = 700;

// =====================================================
// TIMING
// =====================================================

unsigned long lastSensorRead = 0;
unsigned long lastMQTTReconnect = 0;

const unsigned long SENSOR_INTERVAL = 3000;

// =====================================================
// RELAY CONTROL
// =====================================================

// YOUR RELAY IS REVERSED
//
// HIGH = FAN ON
// LOW  = FAN OFF

void fanON()
{
    digitalWrite(RELAY_PIN, HIGH);
    fanState = true;
}

void fanOFF()
{
    digitalWrite(RELAY_PIN, LOW);
    fanState = false;
}

// =====================================================
// OLED
// =====================================================

void updateOLED()
{
    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.println("INDUSTRIAL IoT");

    display.drawLine(
        0, 10,
        127, 10,
        SSD1306_WHITE
    );

    display.setCursor(0, 16);
    display.print("Temp: ");
    display.print(temperature, 1);
    display.println(" C");

    display.setCursor(0, 28);
    display.print("Humidity: ");
    display.print(humidity, 1);
    display.println("%");

    display.setCursor(0, 40);
    display.print("Gas: ");
    display.println(gasValue);

    display.setCursor(0, 52);
    display.print("Fan: ");
    display.print(fanState ? "ON" : "OFF");

    display.print(" ");
    display.print(autoMode ? "AUTO" : "MAN");

    display.display();
}

// =====================================================
// READ SENSORS
// =====================================================

void readSensors()
{
    float newTemperature =
        dht.readTemperature();

    float newHumidity =
        dht.readHumidity();

    if (!isnan(newTemperature))
    {
        temperature = newTemperature;
    }

    if (!isnan(newHumidity))
    {
        humidity = newHumidity;
    }

    // Read MQ-2
    gasValue = analogRead(MQ2_PIN);

    // =================================================
    // AUTOMATIC FAN CONTROL
    // =================================================

    if (autoMode)
    {
        // FAN ON if EITHER condition is true
        //
        // Temperature > 30°C
        // OR
        // Gas > 700

        if (
            temperature > FAN_ON_TEMP ||
            gasValue > GAS_THRESHOLD
        )
        {
            fanON();
        }
        else
        {
            fanOFF();
        }
    }

    updateOLED();

    Serial.println();
    Serial.println("==============================");

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("MQ-2: ");
    Serial.println(gasValue);

    Serial.print("Fan: ");
    Serial.println(
        fanState ? "ON" : "OFF"
    );

    Serial.print("Mode: ");
    Serial.println(
        autoMode ? "AUTO" : "MANUAL"
    );

    Serial.println("==============================");
}

// =====================================================
// SEND MQTT TELEMETRY
// =====================================================

void publishTelemetry()
{
    char payload[300];

    snprintf(
        payload,
        sizeof(payload),
        "{\"temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"gas\":%d,"
        "\"fan\":\"%s\","
        "\"mode\":\"%s\"}",
        temperature,
        humidity,
        gasValue,
        fanState ? "ON" : "OFF",
        autoMode ? "AUTO" : "MANUAL"
    );

    mqttClient.publish(
        TOPIC_TELEMETRY,
        payload
    );

    Serial.println("Telemetry sent:");
    Serial.println(payload);
}

// =====================================================
// MQTT COMMAND RECEIVER
// =====================================================

void mqttCallback(
    char* topic,
    byte* payload,
    unsigned int length
)
{
    String command = "";

    for (
        unsigned int i = 0;
        i < length;
        i++
    )
    {
        command += (char)payload[i];
    }

    command.trim();
    command.toUpperCase();

    Serial.print("MQTT Command: ");
    Serial.println(command);

    // ================================================
    // AUTO
    // ================================================

    if (command == "AUTO")
    {
        autoMode = true;

        // Apply both automatic thresholds immediately

        if (
            temperature > FAN_ON_TEMP ||
            gasValue > GAS_THRESHOLD
        )
        {
            fanON();
        }
        else
        {
            fanOFF();
        }
    }

    // ================================================
    // MANUAL FAN ON
    // ================================================

    else if (command == "ON")
    {
        autoMode = false;

        fanON();
    }

    // ================================================
    // MANUAL FAN OFF
    // ================================================

    else if (command == "OFF")
    {
        autoMode = false;

        fanOFF();
    }

    updateOLED();

    // Publish immediate status
    publishTelemetry();

    Serial.print("Mode: ");
    Serial.println(
        autoMode ? "AUTO" : "MANUAL"
    );

    Serial.println("==============================");
}

// =====================================================
// CONNECT TO WIFI
// =====================================================

void connectWiFi()
{
    Serial.println();
    Serial.print("Connecting to Wi-Fi");

    WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
    );

    while (
        WiFi.status() != WL_CONNECTED
    )
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal Strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
}

// =====================================================
// CONNECT TO HIVEMQ
// =====================================================

void connectMQTT()
{
    if (mqttClient.connected())
    {
        return;
    }

    Serial.println();
    Serial.println("Connecting to HiveMQ...");

    String clientID =
        "ESP32_Industrial_"
        + String(
            (uint32_t)ESP.getEfuseMac(),
            HEX
        );

    if (
        mqttClient.connect(
            clientID.c_str(),
            MQTT_USERNAME,
            MQTT_PASSWORD
        )
    )
    {
        Serial.println(
            "HiveMQ Connected!"
        );

        // Subscribe to commands
        mqttClient.subscribe(
            TOPIC_COMMAND
        );

        mqttClient.publish(
            TOPIC_STATUS,
            "ESP32 ONLINE"
        );

        publishTelemetry();
    }
    else
    {
        Serial.print(
            "MQTT connection failed. State: "
        );

        Serial.println(
            mqttClient.state()
        );
    }
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();

    Serial.println(
        "================================"
    );

    Serial.println(
        " INDUSTRIAL IoT AUTOMATION"
    );

    Serial.println(
        " ESP32 + HiveMQ Cloud"
    );

    Serial.println(
        "================================"
    );

    // =================================================
    // RELAY
    // =================================================

    pinMode(
        RELAY_PIN,
        OUTPUT
    );

    // Start with fan OFF
    fanOFF();

    // =================================================
    // MQ2
    // =================================================

    pinMode(
        MQ2_PIN,
        INPUT
    );

    // =================================================
    // DHT
    // =================================================

    dht.begin();

    // =================================================
    // OLED
    // =================================================

    Wire.begin(
        OLED_SDA,
        OLED_SCL
    );

    if (
        !display.begin(
            SSD1306_SWITCHCAPVCC,
            0x3C
        )
    )
    {
        Serial.println(
            "OLED not found!"
        );
    }
    else
    {
        display.clearDisplay();

        display.setTextSize(1);

        display.setTextColor(
            SSD1306_WHITE
        );

        display.setCursor(10, 20);

        display.println(
            "Industrial IoT"
        );

        display.setCursor(20, 35);

        display.println(
            "Starting..."
        );

        display.display();

        delay(2000);
    }

    // =================================================
    // Wi-Fi
    // =================================================

    connectWiFi();

    // =================================================
    // TLS
    // =================================================

    secureClient.setInsecure();

    // =================================================
    // MQTT
    // =================================================

    mqttClient.setServer(
        MQTT_HOST,
        MQTT_PORT
    );

    mqttClient.setCallback(
        mqttCallback
    );

    mqttClient.setBufferSize(
        512
    );

    // =================================================
    // CONNECT MQTT
    // =================================================

    connectMQTT();
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    // =================================================
    // KEEP MQTT CONNECTION ALIVE
    // =================================================

    if (!mqttClient.connected())
    {
        unsigned long now =
            millis();

        if (
            now - lastMQTTReconnect > 5000
        )
        {
            lastMQTTReconnect = now;

            connectMQTT();
        }
    }
    else
    {
        mqttClient.loop();
    }

    // =================================================
    // READ SENSORS PERIODICALLY
    // =================================================

    unsigned long now =
        millis();

    if (
        now - lastSensorRead >=
        SENSOR_INTERVAL
    )
    {
        lastSensorRead = now;

        readSensors();

        if (mqttClient.connected())
        {
            publishTelemetry();
        }
    }
}
