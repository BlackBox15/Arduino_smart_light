#include <NewPing.h>

#define ULTRASONIC_TRIGGER_PIN 2
#define ULTRASONIC_ECHO_PIN 3
#define ULTRASONIC_CHECK_PERIOD 1000

NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
int previousMillis;

void setup() { Serial.begin(9600); }

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= ULTRASONIC_CHECK_PERIOD) {
        // запустить опрос УЗ датчика
        previousMillis = currentMillis;
    }
}
