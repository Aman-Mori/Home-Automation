#define BLYNK_TEMPLATE_ID "TMPL30OJUAcwk"
#define BLYNK_TEMPLATE_NAME "home automation advance"
#define BLYNK_AUTH_TOKEN "kxFHz5iFu70_5rn5wb6RiKtbNlklYINW"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// WiFi credentials
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "iot";
char pass[] = ".........";

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
#define TEMP_VPIN V7
#define LDR_VPIN V8

// DHT setup
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// State tracking
bool fanIsOn = false;
bool lightIsOn = false;
unsigned long lastFanChange = 0;
unsigned long lastLightChange = 0;

// Thresholds
#define TEMP_ON 33
#define TEMP_OFF 31
#define LDR_ON 2100
#define LDR_OFF 1900
#define CHANGE_DELAY 10000  // 10 sec delay after state change

// Update Blynk dashboard
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
  lastFanChange = millis();
  updateFanDashboard();
}

BLYNK_WRITE(LIGHT_SWITCH_VPIN) {
  int state = param.asInt();
  lightIsOn = (state == 1);
  digitalWrite(LIGHT_PIN, lightIsOn ? LOW : HIGH);
  lastLightChange = millis();
  updateLightDashboard();
}

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

void loop() {
  Blynk.run();

  float temp = dht.readTemperature();
  int ldrValue = analogRead(LDR_PIN);

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" °C  |  LDR: "); Serial.println(ldrValue);

  Blynk.virtualWrite(TEMP_VPIN, temp);
  Blynk.virtualWrite(LDR_VPIN, ldrValue);

  unsigned long now = millis();

  // FAN Auto Control with hysteresis and 10-sec delay
  if (now - lastFanChange > CHANGE_DELAY) {
    if (temp > TEMP_ON && !fanIsOn) {
      fanIsOn = true;
      digitalWrite(FAN_PIN, LOW);
      lastFanChange = now;
      updateFanDashboard();
    } else if (temp < TEMP_OFF && fanIsOn) {
      fanIsOn = false;
      digitalWrite(FAN_PIN, HIGH);
      lastFanChange = now;
      updateFanDashboard();
    }
  }

  // LIGHT Auto Control with hysteresis and 10-sec delay
  if (now - lastLightChange > CHANGE_DELAY) {
    if (ldrValue > LDR_ON && !lightIsOn) {
      lightIsOn = true;
      digitalWrite(LIGHT_PIN, LOW);
      lastLightChange = now;
      updateLightDashboard();
    } else if (ldrValue < LDR_OFF && lightIsOn) {
      lightIsOn = false;
      digitalWrite(LIGHT_PIN, HIGH);
      lastLightChange = now;
      updateLightDashboard();
    }
  }

  delay(500);
}
