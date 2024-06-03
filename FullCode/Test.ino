//esp
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

// Device Code
String deviceCode = "Device01";

// Wifi Configuration
const char ssid[] = "hgp";
const char pass[] = "123456788";

WiFiClient net;

// Define thresholds for Goat
const int goatBreathMin = 26;
const int goatBreathMax = 54;
const int goatHeartRateMin = 70;
const int goatHeartRateMax = 80;
const float goatTempMin = 38.5;
const float goatTempMax = 39.7;

// Define thresholds for Sheep
const int sheepBreathMin = 26;
const int sheepBreathMax = 32;
const int sheepHeartRateMin = 70;
const int sheepHeartRateMax = 80;
const float sheepTempMin = 38.5;
const float sheepTempMax = 39.0;

// Millis
unsigned long lastPublishMillis = 0;
unsigned long lastBreathMillis = 0;
unsigned long lastHeartRateMillis = 0;
unsigned long lastTemperatureMillis = 0;
unsigned long lastPredictionMillis = 0;

// Interval dalam milidetik
const unsigned long publishInterval = 1000;
const unsigned long breathInterval = 5000;
const unsigned long heartRateInterval = 3000;
const unsigned long temperatureInterval = 4000;
const unsigned long predictionInterval = 6000;

// ds18b20
#include <OneWire.h>
#include <DallasTemperature.h>
const int oneWireBus = 2;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// max
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
MAX30105 particleSensor;
const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

// napas
#define AMBANG_BATAS_DETEKSI 100 // Nilai ambang batas untuk mendeteksi suara
#define AMBANG_BATAS_TIDAK_DETEKSI 4095 // Nilai ambang batas saat tidak ada suara

// Define state constants for the loop
#define MAX_SENSOR 0
#define TEMP_SENSOR 1
#define BREATH_SENSOR 2

int currentState = MAX_SENSOR;
unsigned long stateStartTime = 0;
const unsigned long maxSensorDuration = 30000; // 30 seconds
const unsigned long tempSensorDuration = 30000; // 30 seconds
const unsigned long breathSensorDuration = 60000; // 1 minute

// Global variables to store sensor values
float globalTemperature = 0;
int globalHeartRate = 0;
int globalBreathRate = 0;
unsigned long soundCount = 0; // Menyimpan jumlah suara yang terdeteksi

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");
  // Start the DS18B20 sensor
  sensors.begin();

  // Initialize MAX30105 sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED

  // Start the DS18B20 sensor
  sensors.begin();

  // Start WiFi
  WiFi.begin(ssid, pass);
  connect();

  stateStartTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  Serial.print("Current State: ");
  Serial.println(currentState);

  switch (currentState) {
    case MAX_SENSOR:
      Serial.println("Running MAX30105 sensor...");
      max();
      if (currentTime - stateStartTime >= maxSensorDuration) {
        globalHeartRate = beatAvg; // Save the average heart rate
        stateStartTime = currentTime;
        currentState = TEMP_SENSOR;
      }
      break;

    case TEMP_SENSOR:
      Serial.println("Running temperature sensor...");
      suhu();
      if (currentTime - stateStartTime >= tempSensorDuration) {
        sensors.requestTemperatures();
        globalTemperature = sensors.getTempCByIndex(0); // Save the temperature
        stateStartTime = currentTime;
        currentState = BREATH_SENSOR;
      }
      break;

    case BREATH_SENSOR:
      Serial.println("Running breath sensor...");
      napas();
      if (currentTime - stateStartTime >= breathSensorDuration) {
        // After running breath sensor, store the global breath rate (assuming soundCount is updated inside napas function)
        globalBreathRate = soundCount;
        stateStartTime = currentTime;

        // Print all sensor values
        Serial.print("Heart Rate (BPM): ");
        Serial.println(globalHeartRate);
        Serial.print("Temperature (°C): ");
        Serial.println(globalTemperature);
        Serial.print("Breath Rate: ");
        Serial.println(globalBreathRate);

        // Perform health prediction
        PredictionHealth();

        // Wait for 10 seconds
        delay(10000);

        // Reset state machine
        stateStartTime = millis();
        currentState = MAX_SENSOR;
      }
      break;
  }
}

void suhu() {
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  float temperatureF = sensors.getTempFByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");
  Serial.print(temperatureF);
  Serial.println("ºF");
}

void max() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true) {
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
      rateSpot %= RATE_SIZE; // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);

  if (irValue < 50000)
    Serial.print(" No finger?");

  Serial.println();
}

void napas() {
  static unsigned long lastMinute = 0; // variabel untuk menyimpan waktu terakhir perhitungan
  static int ambangBatas = AMBANG_BATAS_TIDAK_DETEKSI; // Nilai ambang batas sensor default

  unsigned long currentMillis = millis(); // mendapatkan waktu saat ini

  // Membaca sensor suara
  int soundValue = analogRead(A0);
  Serial.println(soundValue);
  // Menyesuaikan nilai ambang batas berdasarkan deteksi suara
  if (soundValue < AMBANG_BATAS_DETEKSI) {
    ambangBatas = AMBANG_BATAS_TIDAK_DETEKSI; // Gunakan nilai ambang batas tinggi saat tidak ada suara terdeteksi
    soundCount++;
  } else {
    ambangBatas = 0; // Ubah nilai ambang batas kembali ke default jika suara terdeteksi
  }

  // Menghitung jumlah suara per menit
  if (currentMillis - lastMinute >= 60000) { // 60000 milliseconds = 1 menit
    float soundPerMinute = (float)soundCount / ((float)(currentMillis - lastMinute) / 60000.0);
    Serial.print("Jumlah suara per menit: ");
    Serial.println(soundPerMinute);

    // Reset jumlah suara dan waktu terakhir perhitungan
    soundCount = 0;
    lastMinute = currentMillis;
  }
}

void PredictionHealth() {
  // Parameter
  String messageGoat = "";
  String messageSheep = "";

  // Prediction for Goat
  if (globalBreathRate >= goatBreathMin && globalBreathRate <= goatBreathMax &&
      globalHeartRate >= goatHeartRateMin && globalHeartRate <= goatHeartRateMax &&
      globalTemperature >= goatTempMin && globalTemperature <= goatTempMax) {
    messageGoat = "Kambing dalam kondisi sehat";
  } else {
    messageGoat = "Kambing dalam kondisi tidak sehat";
  }

  Serial.println(messageGoat);

  // Prediction for Sheep
  if (globalBreathRate >= sheepBreathMin && globalBreathRate <= sheepBreathMax &&
      globalHeartRate >= sheepHeartRateMin && globalHeartRate <= sheepHeartRateMax &&
      globalTemperature >= sheepTempMin && globalTemperature <= sheepTempMax) {
    messageSheep = "Domba dalam kondisi sehat";
  } else {
    messageSheep = "Domba dalam kondisi tidak sehat";
  }

  Serial.println(messageSheep);
}