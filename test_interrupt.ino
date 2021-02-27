#include <LowPower.h>

const byte ledPin = LED_BUILTIN;
const byte interruptPin = 2;
volatile byte state = LOW;
int awakePin = 13;

void setup() {
  Serial.begin(9600);
  Serial.println("Before interrupt");
  pinMode(awakePin, OUTPUT);    // sets the digital pin 13 as output

  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
}

void loop() {
  digitalWrite(ledPin, state);
  Serial.println("Not in sleep");
  digitalWrite(awakePin, HIGH); // sets the digital pin 13 on
  delay(1000);
  digitalWrite(awakePin, LOW); // sets the digital pin 13 on
LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void blink() {
  // do nothing. Just wakes up arduino
}
