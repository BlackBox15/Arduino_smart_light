#include <NewPing.h>
#include <SD.h>

#define ULTRASONIC_TRIGGER_PIN 2
#define ULTRASONIC_ECHO_PIN 3
#define ULTRASONIC_CHECK_PERIOD 3000

#define SD_CHIP_SELECT_PIN 10

NewPing UltraSonicSensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
unsigned long previousMillis;
unsigned long ultrasonicDistance;

// =================================================================//
// 																	//
//								setup								//
// 																	//
// =================================================================//
void setup() {
    SD.begin(SD_CHIP_SELECT_PIN);

    Serial.begin(9600);
    while (!Serial);
}
// =================================================================//
// 																	//
//								loop								//
// 																	//
// =================================================================//
void loop() {
    unsigned long currentMillis = millis();
	
	// periodic ultrasonig measurement
    if (currentMillis - previousMillis >= ULTRASONIC_CHECK_PERIOD) {
		ultrasonicDistance = UltraSonicSensor.ping_cm();
        previousMillis = currentMillis;
    }
}
