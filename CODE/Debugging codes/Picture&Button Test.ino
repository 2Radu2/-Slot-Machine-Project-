void setup() {
  pinMode(A0, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(A0) == LOW) {
    Serial.println("Button on A0 is pressed!");
    delay(200);
  }

  if (digitalRead(6) == LOW) {
    Serial.println("Button on D6 is pressed!");
    delay(200);
  }

  if (digitalRead(12) == LOW) {
    Serial.println("Button on D12 is pressed!");
    delay(200);
  }
}
