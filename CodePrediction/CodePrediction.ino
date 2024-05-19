// Define sensor pins
const int breathSensorPin = A0; // Analog pin for breath sensor
const int heartRateSensorPin = A1; // Analog pin for heart rate sensor
const int tempSensorPin = A2; // Analog pin for temperature sensor

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

void setup() {
  Serial.begin(9600);
  pinMode(breathSensorPin, INPUT);
  pinMode(heartRateSensorPin, INPUT);
  pinMode(tempSensorPin, INPUT);
}

void loop() {
  int breathRate = analogRead(breathSensorPin); // Replace with actual breath rate calculation
  int heartRate = analogRead(heartRateSensorPin); // Replace with actual heart rate calculation
  float temperature = analogRead(tempSensorPin) * (5.0 / 1023.0) * 100; // Example conversion for temp sensor
  
  Serial.print("Breath Rate: ");
  Serial.println(breathRate);
  Serial.print("Heart Rate: ");
  Serial.println(heartRate);
  Serial.print("Temperature: ");
  Serial.println(temperature);

  // Prediction for Goat
  if (breathRate >= goatBreathMin && breathRate <= goatBreathMax &&
      heartRate >= goatHeartRateMin && heartRate <= goatHeartRateMax &&
      temperature >= goatTempMin && temperature <= goatTempMax) {
    Serial.println("Goat is healthy");
  } else {
    Serial.println("Goat might be unhealthy");
  }

  // Prediction for Sheep
  if (breathRate >= sheepBreathMin && breathRate <= sheepBreathMax &&
      heartRate >= sheepHeartRateMin && heartRate <= sheepHeartRateMax &&
      temperature >= sheepTempMin && temperature <= sheepTempMax) {
    Serial.println("Sheep is healthy");
  } else {
    Serial.println("Sheep might be unhealthy");
  }

  delay(1000); // Wait for 1 second before next reading
}
