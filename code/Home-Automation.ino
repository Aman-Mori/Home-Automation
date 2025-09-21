#define BLYNK_TEMPLATE_ID "TMPL30OJUAcwk"
#define BLYNK_TEMPLATE_NAME "home automation advance"
#define BLYNK_AUTH_TOKEN "kxFHz5iFu70_5rn5wb6RiKtbNlklYINW"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "iot-1";
char pass[] = "Hello@123";

// GPIO Pins
#define FAN_PIN 25
#define LIGHT_PIN 26
#define DHTPIN 27
#define LDR_PIN 34

// Blynk Virtual Pins
#define FAN_SWITCH_VPIN V1
#define LIGHT_SWITCH_VPIN V2
#define FAN_LED_ON_VPIN V3
#define FAN_LED_OFF_VPIN V4
#define LIGHT_LED_ON_VPIN V5
#define LIGHT_LED_OFF_VPIN V6
#define TEMP_VPIN V7   // Temperature gauge/display
#define LDR_VPIN V8    // LDR gauge/display

// DHT setup
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// State tracking
bool fanIsOn = false;
bool lightIsOn = false;
unsigned long lastManualFan = 0;
unsigned long lastManualLight = 0;

// Thresholds
#define LDR_THRESHOLD 2000
#define TEMP_ON 28
#define TEMP_OFF 27
#define MANUAL_TIMEOUT 10000  // 10 sec ignore auto after manual

// Update LEDs & Switch
void updateFanDashboard() {
  Blynk.virtualWrite(FAN_SWITCH_VPIN, fanIsOn ? 1 : 0);
  Blynk.virtualWrite(FAN_LED_ON_VPIN, fanIsOn ? 255 : 0);
  Blynk.virtualWrite(FAN_LED_OFF_VPIN, fanIsOn ? 0 : 255);
}

void updateLightDashboard() {
  Blynk.virtualWrite(LIGHT_SWITCH_VPIN, lightIsOn ? 1 : 0);
  Blynk.virtualWrite(LIGHT_LED_ON_VPIN, lightIsOn ? 255 : 0);
  Blynk.virtualWrite(LIGHT_LED_OFF_VPIN, lightIsOn ? 0 : 255);
}

// Manual Control
BLYNK_WRITE(FAN_SWITCH_VPIN) {
  int state = param.asInt();
  fanIsOn = (state == 1);
  digitalWrite(FAN_PIN, fanIsOn ? LOW : HIGH);
  lastManualFan = millis();
  updateFanDashboard();
}

BLYNK_WRITE(LIGHT_SWITCH_VPIN) {
  int state = param.asInt();
  lightIsOn = (state == 1);
  digitalWrite(LIGHT_PIN, lightIsOn ? LOW : HIGH);
  lastManualLight = millis();
  updateLightDashboard();
}

// Setup
void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  dht.begin();

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  digitalWrite(FAN_PIN, HIGH);   // OFF
  digitalWrite(LIGHT_PIN, HIGH); // OFF

  updateFanDashboard();
  updateLightDashboard();
}

// Loop
void loop() {
  Blynk.run();

  float temp = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" °C  |  LDR: "); Serial.println(ldrValue);

  // Send temp and LDR to dashboard
  Blynk.virtualWrite(TEMP_VPIN, temp);
  Blynk.virtualWrite(LDR_VPIN, ldrValue);

  unsigned long now = millis();

  // FAN Auto Control
  if (now - lastManualFan > MANUAL_TIMEOUT) {
    if (temp > TEMP_ON && !fanIsOn) {
      fanIsOn = true;
      digitalWrite(FAN_PIN, LOW);
      updateFanDashboard();
    } else if (temp < TEMP_OFF && fanIsOn) {
      fanIsOn = false;
      digitalWrite(FAN_PIN, HIGH);
      updateFanDashboard();
    }
  }

  // LIGHT Auto Control
  if (now - lastManualLight > MANUAL_TIMEOUT) {
    if (ldrValue > LDR_THRESHOLD && !lightIsOn) {
      lightIsOn = true;
      digitalWrite(LIGHT_PIN, LOW);
      updateLightDashboard();
    } else if (ldrValue <= LDR_THRESHOLD && lightIsOn) {
      lightIsOn = false;
      digitalWrite(LIGHT_PIN, HIGH);
      updateLightDashboard();
    }
  }

  delay(1000);
}
