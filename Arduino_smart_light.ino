#include <NewPing.h>

#define ULTRASONIC_TRIGGER_PIN 2
#define ULTRASONIC_ECHO_PIN 3
#define ULTRASONIC_CHECK_PERIOD 3000

NewPing UltraSonicSensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
unsigned long previousMillis;

// ==================================================================
// 
//                                  setup
// 
// ==================================================================
void setup() { 
    Serial.begin(9600);
    while (!Serial);
}

// ==================================================================
// 
//                              loop
// 
// ==================================================================
void loop() {
    unsigned long currentMillis = millis();
	
	// periodic ultrasonig measurement
    if (currentMillis - previousMillis >= ULTRASONIC_CHECK_PERIOD) {
        previousMillis = currentMillis;
    }
}
