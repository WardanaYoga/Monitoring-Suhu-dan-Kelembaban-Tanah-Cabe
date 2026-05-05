#define RELAY_PIN 26

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // relay mati awal
  Serial.begin(115200);
}

void loop() {
  // Motor ON
  digitalWrite(RELAY_PIN, HIGH);
  Serial.println("Pompa On");
  delay(3000);

  // Motor OFF
  digitalWrite(RELAY_PIN, LOW);
  Serial.println("Pompa Off");
  delay(3000);
}
