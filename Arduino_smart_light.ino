#include <NewPing.h>
#include <SD.h>
#include <Arduino.h>
#include <Ds1302.h>
#include <EEPROM.h>
#include <FastLED.h>

#define ULTRASONIC_1_TRIGGER_PIN      2
#define ULTRASONIC_1_ECHO_PIN         3
#define ULTRASONIC_2_TRIGGER_PIN      4
#define ULTRASONIC_2_ECHO_PIN         9
#define ULTRASONIC_CHECK_PERIOD_MS  1000
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

#define PIN_RST         7  // Пин для управления
#define PIN_DAT         6  // Пин для управления
#define PIN_CLK         5  // Пин для управления
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);

#define LED_PIN         A1  // Пин управления лентой
#define LED_NUM         15  // количество LED'ов, управляемых одним чипом
CRGB leds[LED_NUM];
bool lightSwitch;

// =================================================================//
// 																	//
//								setup								//
// 																	//
// =================================================================//
void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
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

    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_NUM);
    // FastLED.setBrightness(10);
    // FastLED.show();

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

    if (!digitalRead(BUTTON_PIN)) {
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

        printTimeFromRtc(now);        

        if (now.hour >= timeSettings[9] || now.hour < timeSettings[5]) {
            digitalWrite(LED_STRIPE_DO, LOW);
            lightSwitch = false;
        } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
            if (distanceFromSensor1 <= ULTRASONIC_SWITCH_DISTANCE || distanceFromSensor2 <= ULTRASONIC_SWITCH_DISTANCE) {
                if (digitalRead(LED_STRIPE_DO)) {
                    digitalWrite(LED_STRIPE_DO, LOW);
                    lightSwitch = false;
                } else {
                    digitalWrite(LED_STRIPE_DO, HIGH);
                    lightSwitch = true;
                }
            }
        } else if (now.hour >= timeSettings[7] || now.hour < timeSettings[9]) {
            digitalWrite(LED_STRIPE_DO, HIGH);
            lightSwitch = true;
        }

        if (lightSwitch) {
            for (int i = 0; i < LED_NUM; i++) {
                leds[i].setRGB(30, 30, 30);   // RGB, 0-255
            }
            FastLED.show();
        } else {
            for (int i = 0; i < LED_NUM; i++) {
                leds[i].setRGB(0, 0, 0);   // RGB, 0-255
            }
            FastLED.clear();
        }

        // printDistance();

        prevDistanceCheck = currentMillis;
    }
}