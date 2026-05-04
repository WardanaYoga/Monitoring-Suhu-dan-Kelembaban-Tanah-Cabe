// === Pin Definition ===
const int SOIL_PIN = 34;  // GPIO ADC

// === Variabel ===
int soilValue = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== TEST SOIL MOISTURE SENSOR ===");

  // Set ADC resolution (optional)
  analogReadResolution(12); // 0 - 4095
}

void loop() {
  soilValue = analogRead(SOIL_PIN);

  Serial.print("Nilai Soil Moisture: ");
  Serial.println(soilValue);

  // Interpretasi sederhana
  if (soilValue > 3000) {
    Serial.println("Kondisi: KERING");
  } 
  else if (soilValue > 2000) {
    Serial.println("Kondisi: LEMBAB");
  } 
  else {
    Serial.println("Kondisi: BASAH");
  }

  Serial.println("------------------------");

  delay(1000);
}
