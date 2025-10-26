#include <NewPing.h>
#include <SD.h>
#include <Arduino.h>
#include <Ds1302.h>
#include <EEPROM.h>

#define ULTRASONIC_1_TRIGGER_PIN      2
#define ULTRASONIC_1_ECHO_PIN         3
#define ULTRASONIC_2_TRIGGER_PIN      4
#define ULTRASONIC_2_ECHO_PIN         9
#define ULTRASONIC_CHECK_PERIOD_MS  2000
#define ULTRASONIC_SWITCH_DISTANCE  10
NewPing UltraSonicSensor1(ULTRASONIC_1_TRIGGER_PIN, ULTRASONIC_1_ECHO_PIN);
NewPing UltraSonicSensor2(ULTRASONIC_2_TRIGGER_PIN, ULTRASONIC_2_ECHO_PIN);

#define BUTTON_PIN                  A0

#define TIME_CHECK_PERIOD_MS        1000
#define LED_STRIPE_DO               8
#define SD_CHIP_SELECT_PIN          10

unsigned long prevDistanceCheck;
unsigned long prevTimeCheck;
unsigned long distanceFromSensor1;
unsigned long distanceFromSensor2;
int timeSettings[11];

// Определение пинов для подключения DS1302
#define PIN_RST         7  // Пин для управления
#define PIN_DAT         6  // Пин для управления
#define PIN_CLK         5  // Пин для управления
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);

// =================================================================//
// 																	//
//								setup								//
// 																	//
// =================================================================//
void setup() {
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_STRIPE_DO, OUTPUT);

    Ds1302::DateTime initTime;

    EEPROM.get(0, timeSettings);
    rtc.init();
    rtc.getDateTime(&initTime);
    timeSettings[0] = initTime.hour;
    timeSettings[1] = initTime.minute;
    timeSettings[2] = initTime.day;
    timeSettings[3] = initTime.month;
    timeSettings[4] = initTime.year;

    Serial.begin(9600);
    while (!Serial);

    // printTimeSettings();
}
// =================================================================//
// 																	//
//								loop								//
// 																	//
// =================================================================//
void loop() {
    unsigned long currentMillis = millis();
    Ds1302::DateTime now;

    if (digitalRead(BUTTON_PIN)) {
        // считывание обновлённых данных с SD-карты
        Serial.println("button pressed..");
        readSdCard();
    }

    if (currentMillis - prevTimeCheck >= TIME_CHECK_PERIOD_MS) {
        rtc.getDateTime(&now);
        prevTimeCheck = currentMillis;
    }
	
    if (currentMillis - prevDistanceCheck >= ULTRASONIC_CHECK_PERIOD_MS) {
        distanceFromSensor1 = UltraSonicSensor1.ping_cm();
        distanceFromSensor2 = UltraSonicSensor2.ping_cm();

        // printTimeFromRtc(now);        

        if (now.hour >= timeSettings[9] || now.hour < timeSettings[5]) {
            digitalWrite(LED_STRIPE_DO, LOW);
        } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
            if (distanceFromSensor1 <= ULTRASONIC_SWITCH_DISTANCE || distanceFromSensor2 <= ULTRASONIC_SWITCH_DISTANCE) {
                if (digitalRead(LED_STRIPE_DO)) {
                    digitalWrite(LED_STRIPE_DO, LOW);
                } else {
                    digitalWrite(LED_STRIPE_DO, HIGH);
                }
            }
        } else if (now.hour >= timeSettings[7] || now.hour < timeSettings[9]) {
            digitalWrite(LED_STRIPE_DO, HIGH);
        }

        // printDistance();

        prevDistanceCheck = currentMillis;
    }
}