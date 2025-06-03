#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h> // Servo motor library

// WiFi configuration
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT configuration
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* pub_temp = "/12345/temp";
const char* pub_hum = "/12345/hum";
const char* subs_led = "/12345/led";
const char* subs_servo = "/12345/servo";
const char* subs_keep_led = "/12345/keep_led";
const char* subs_keep_servo = "/12345/keep_servo";

// MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// DHT22 sensor configuration
#define DHTPIN 16
#define DHTTYPE DHT22
DHTesp dhtSensor;

// LCD configuration
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Servo motor configuration
Servo myServo;
#define SERVO_PIN 17

// Relay, button configuration
#define RELAY_PIN 15
#define BUTTON_PIN 18

// Auto mode and control state
bool ledState = false;
bool servoState = false;
bool autoMode = true;
bool isManualControl = false; // Flag to check if manual control is active
bool keepLed = false; // Flag to keep LED state
bool keepServo = false; // Flag to keep Servo state
bool continuousServo = false; // Flag to toggle continuous servo rotation

// Temperature and humidity thresholds
#define TEMP_MIN 10.0
#define TEMP_MAX 50.0
#define HUM_MIN 10.0
#define HUM_MAX 50.0

unsigned long lastMsg = 0;
const long interval = 1000;

void setup() {
  Serial.begin(115200);

  // Initialize WiFi and MQTT
  setup_wifi();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  // Initialize LCD, servo, and DHT sensor
  lcd.init();
  lcd.backlight();
  myServo.attach(SERVO_PIN);
  dhtSensor.setup(DHTPIN, DHTesp::DHT22);

  // Set pin modes
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, LOW); // Ensure relay is off initially
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT broker");
      mqttClient.subscribe(subs_led);
      mqttClient.subscribe(subs_servo);
      mqttClient.subscribe(subs_keep_led);
      mqttClient.subscribe(subs_keep_servo);
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String strMsg;

  for (int i = 0; i < length; i++) {
    strMsg += (char)message[i];
  }
  Serial.println(strMsg);

  if (String(topic) == subs_led) {
    if (strMsg == "ON") {
      ledState = true;
      digitalWrite(RELAY_PIN, HIGH);
    } else if (strMsg == "OFF") {
      ledState = false;
      digitalWrite(RELAY_PIN, LOW);
    }
  } else if (String(topic) == subs_servo) {
    if (strMsg == "ON") {
      servoState = true;
      isManualControl = true; // Set manual control flag
    } else if (strMsg == "OFF") {
      servoState = false;
      myServo.write(0); // Stop servo
    }
  } else if (String(topic) == subs_keep_servo) {
    keepServo = (strMsg == "ON");
  } else if (String(topic) == subs_keep_led) {
    keepLed = (strMsg == "ON");
  }
}

void loop() {
  if (!mqttClient.connected()) {
    mqttConnect();
  }
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Read temperature and humidity from DHT sensor
    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    if (!isnan(data.temperature) && !isnan(data.humidity)) {
      // Publish temperature and humidity data to MQTT
      char tempBuffer[8];
      char humBuffer[8];
      dtostrf(data.temperature, 1, 2, tempBuffer);
      dtostrf(data.humidity, 1, 2, humBuffer);
      mqttClient.publish(pub_temp, tempBuffer);
      mqttClient.publish(pub_hum, humBuffer);

      // Display data on LCD
      lcd.setCursor(0, 0);
      lcd.print("Temp: " + String(data.temperature, 2) + "*C");
      lcd.setCursor(0, 1);
      lcd.print("Humidity: " + String(data.humidity, 1) + "%");

      // Control servo based on temperature and humidity if not in manual mode
      if (autoMode && !isManualControl && !keepServo) {
        if (data.temperature >= TEMP_MIN && data.temperature <= TEMP_MAX && data.humidity >= HUM_MIN && data.humidity <= HUM_MAX) {
          myServo.write(90);  // Rotate servo to 90 degrees
          delay(300);         // Small delay for stability
          myServo.write(0);   // Return servo to 0 degrees
        } else {
          myServo.write(0);   // Keep servo at 0 degrees if out of range
        }
      }

      // Control servo based on keepServo flag if manual control is active
      if (servoState && keepServo) {
        myServo.write(90);  // Rotate servo to 90 degrees
        delay(300);         // Small delay for stability
        myServo.write(0);   // Return servo to 0 degrees
      }
    }
  }

  // Button handling for manual relay control
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      ledState = !ledState;  // Toggle LED state
      autoMode = false;      // Disable auto mode
      digitalWrite(RELAY_PIN, ledState ? HIGH : LOW);

      myServo.write(90);  // Rotate servo to 90 degrees
      delay(300);         // Small delay for stability
      myServo.write(0);
      // Wait for button release
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);
      }
    }
    }

  // Auto mode for relay control based on humidity if not in manual mode
  if (autoMode && !isManualControl && !keepLed) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    if (!isnan(data.humidity)) {
      if (data.humidity < 40.0) {
        digitalWrite(RELAY_PIN, HIGH);  // Turn on relay if humidity is below threshold
      } else if (data.humidity > 60.0) {
        digitalWrite(RELAY_PIN, LOW);   // Turn off relay if humidity is above threshold
      }
    }
  }

  delay(1000);  // Small delay between loop iterations
}
