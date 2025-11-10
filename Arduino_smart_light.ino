#include <NewPing.h>
#include <SD.h>
#include <Arduino.h>
#include <Ds1302.h>
#include <EEPROM.h>
#include <FastLED.h>

#define ULTRASONIC_1_TRIGGER_PIN        2
#define ULTRASONIC_1_ECHO_PIN           3
#define ULTRASONIC_2_TRIGGER_PIN        4
#define ULTRASONIC_2_ECHO_PIN           9
#define ULTRASONIC_CHECK_PERIOD_MS      1000    // период опроса УЗ-датчика, мс
#define ULTRASONIC_SWITCH_DISTANCE      10      // дистанция срабатывания УЗ-датчика, см
NewPing UltraSonicSensor1(ULTRASONIC_1_TRIGGER_PIN, ULTRASONIC_1_ECHO_PIN);
NewPing UltraSonicSensor2(ULTRASONIC_2_TRIGGER_PIN, ULTRASONIC_2_ECHO_PIN);

#define BUTTON_PIN                      A0

#define TIME_CHECK_PERIOD_MS            1000
#define TEST_LED                        8
#define SD_CHIP_SELECT_PIN              10

unsigned char timeParams[11];   

#define PIN_RST                         7       // Пин для управления
#define PIN_DAT                         6       // Пин для управления
#define PIN_CLK                         5       // Пин для управления
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);

#define LED_PIN                         A1      // Пин управления лентой
#define LED_NUM                         15      // количество чипов (для WS2811 каждый чип управляет 3мя диодами)
#define LED_BRIGHTNESS_STBY             15     // яркость диодов во включенном дежурном режиме
#define LED_BRIGHTNESS_SENSOR           15     // яркость диодов во включенном состоянии по сенсору

#define BUTTON_FILTER_TIME_MS           200     // настройки фильтрации дребезга кнопки, мс
CRGB leds[LED_NUM];
bool lightSwitch;
// =================================================================//
// 																	//
//								setup								//
// 																	//
// =================================================================//
void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TEST_LED, OUTPUT);

    Serial.begin(9600);
    while (!Serial);

    Ds1302::DateTime initTime;

    // EEPROM.get(0, timeSettings);
    rtc.init();
    // rtc.getDateTime(&initTime);
    // timeSettings[0] = initTime.hour;
    // timeSettings[1] = initTime.minute;
    // timeSettings[2] = initTime.day;
    // timeSettings[3] = initTime.month;
    // timeSettings[4] = initTime.year;

    for (int i = 0; i < 11; i++) {
        timeParams[i] = 0;
    }

    // EEPROM.put(0, timeParams);

    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_NUM);
    FastLED.setBrightness(LED_BRIGHTNESS_STBY);
    
    // FastLED.show();
    // printTimeSettings();

    // Serial.println();
    // Serial.println(F("timeParams:"));
    // for (int i = 0; i < 11; i++) {
    //     Serial.print("[");
    //     Serial.print(i);
    //     Serial.print("] = ");
    //     // Serial.println(timeParams[i]);
    //     Serial.println(i);
    // }
}
// =================================================================//
// 																	//
//								loop								//
// 																	//
// =================================================================//
void loop() {
    Ds1302::DateTime now;
    unsigned long currentMillis = millis();
    static unsigned long prevTimeCheck;
    static bool blockReadButton;
    static unsigned long buttonPressTimestamp;
    static unsigned long prevDistanceCheck;
    static unsigned char distanceFromSensor1;
    static unsigned char distanceFromSensor2;
    static String currentState = "Init";


    if (!digitalRead(BUTTON_PIN) && !blockReadButton) {
        Serial.println(F("Button pressed..."));
        buttonPressTimestamp = millis();
        blockReadButton = !blockReadButton;
        readSdCard();
    }

    if (currentMillis - buttonPressTimestamp >= BUTTON_FILTER_TIME_MS) {
        blockReadButton = false;
    }

    if (currentMillis - prevTimeCheck >= TIME_CHECK_PERIOD_MS) {
        rtc.getDateTime(&now);
        prevTimeCheck = currentMillis;
        printTimeFromRtc(now);

        Serial.print(F("current state "));
        Serial.println(currentState);

        Serial.print(F("now.hour "));
        Serial.println(now.hour);
        Serial.print(F("timeSettings[9]"));
        Serial.println(timeParams[9]);
        Serial.print(F("timeSettings[5]"));
        Serial.println(timeParams[5]);
        Serial.print(F("timeSettings[7]"));
        Serial.println(timeParams[7]);


        Serial.print(F("distance: "));
        Serial.print(distanceFromSensor1);
        Serial.print(F(" cm\t"));
        Serial.print(F("distance: "));
        Serial.print(distanceFromSensor2);
        Serial.println(F(" cm"));

    }

    /*

        Описание уставок:

            0		timeSettings[9]		timeSettings[5]		timeSettings[7]		24
            |--------|-------------------|-------------------|------------------|
                stby		off					sensor			stby

    */

    if (now.hour >= timeParams[5] && now.hour <= timeParams[7]) {
        distanceFromSensor1 = UltraSonicSensor1.ping_cm();
        distanceFromSensor2 = UltraSonicSensor2.ping_cm();
        if (distanceFromSensor1 <= ULTRASONIC_SWITCH_DISTANCE || distanceFromSensor2 <= ULTRASONIC_SWITCH_DISTANCE) {
            currentState = "Sensor On";

            for (unsigned char i = 0; i < LED_NUM; i++) {
                leds[i] = CRGB::Red;
            }
            FastLED.show();
            digitalWrite(TEST_LED, HIGH);
        } else {
            currentState = "Sensor Off";
            for (unsigned char i = 1; i < LED_NUM-1; i++) {
                leds[i] = CRGB::Black;
            }
            FastLED.show();
            digitalWrite(TEST_LED, LOW);
        }
        digitalWrite(TEST_LED, HIGH);
    } else if (now.hour > timeParams[7] && now.hour < timeParams[9]) {
        currentState = "Stby";
        leds[0] = CRGB::Red;
        leds[LED_NUM-1] = CRGB::WHITE;
        FastLED.show();
        digitalWrite(TEST_LED, HIGH);
    } else {
        currentState = "Daylight Off";
        for (unsigned char i = 0; i < LED_NUM; i++) {
                leds[i] = CRGB::Black;
            }
        FastLED.show();
        digitalWrite(TEST_LED, LOW);
    }

    // if (now.hour < timeParams[9]) {
        
    // } else if (now.hour > timeParams[9] && now.hour < timeParams[5]) {
    //     // disableLights();
        
    // } else if (now.hour >= timeParams[5] && now.hour < timeParams[7]) {
        
    // } else {
    //     // stby
    //     Serial.println(F("Stby mode 2"));

    //     leds[0] = CRGB::Red;
    //     leds[LED_NUM-1] = CRGB::Red;
    //     FastLED.show();
    //     digitalWrite(TEST_LED, HIGH);
    // }

	
    // if (currentMillis - prevDistanceCheck >= ULTRASONIC_CHECK_PERIOD_MS) {
    //     distanceFromSensor1 = UltraSonicSensor1.ping_cm();
    //     distanceFromSensor2 = UltraSonicSensor2.ping_cm();

    //     printTimeFromRtc(now);

    
    //     if (now.hour < timeSettings[9]) {
    //         enableStandbyMode();
    //         digitalWrite(LED_STRIPE_DO, HIGH);
    //     } else if (now.hour > timeSettings[9] && now.hour < timeSettings[5]) {
    //         disableLights();
    //         digitalWrite(LED_STRIPE_DO, LOW);
    //     } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
    //         enableSensorMode(int sensorDistance, int distanceCheckValue, int chipsOnLowLevel, int chipsOnTopLevel)
    //         digitalWrite(LED_STRIPE_DO, HIGH);
    //     } else {
    //         enableStandbyMode();
    //         digitalWrite(LED_STRIPE_DO, HIGH);
    //     }

    //     if (now.hour >= timeSettings[9] || now.hour < timeSettings[5]) {
    //         digitalWrite(LED_STRIPE_DO, LOW);
    //         lightSwitch = false;
    //     } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
    //         if (distanceFromSensor1 <= ULTRASONIC_SWITCH_DISTANCE || distanceFromSensor2 <= ULTRASONIC_SWITCH_DISTANCE) {
    //             if (digitalRead(LED_STRIPE_DO)) {
    //                 digitalWrite(LED_STRIPE_DO, LOW);
    //                 lightSwitch = false;
    //             } else {
    //                 digitalWrite(LED_STRIPE_DO, HIGH);
    //                 lightSwitch = true;
    //             }
    //         }
    //     } else if (now.hour >= timeSettings[7] || now.hour < timeSettings[9]) {
    //         digitalWrite(LED_STRIPE_DO, HIGH);
    //         lightSwitch = true;
    //     }

    //     if (lightSwitch) {
    //         for (int i = 0; i < LED_NUM; i++) {
    //             leds[i].setRGB(30, 30, 30);   // RGB, 0-255
    //         }
    //         FastLED.show();
    //     } else {
    //         for (int i = 0; i < LED_NUM; i++) {
    //             leds[i].setRGB(0, 0, 0);   // RGB, 0-255
    //         }
    //         FastLED.clear();
    //     }

    //     // printDistance();

    //     prevDistanceCheck = currentMillis;
    // }
}

void enableStandbyMode(unsigned char chipsOnLowLevel, unsigned char chipsOnTopLevel) {
    FastLED.clear();
    for (unsigned char i = 0; chipsOnLowLevel; i++) {
        leds[i] = CRGB(LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR);
    }
    // for (unsigned char i = LED_NUM; LED_NUM - chipsOnTopLevel; i--) {
    //     leds[i] = CRGB(LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR);
    // }
    FastLED.show();
}

// void enableSensorMode(int sensorDistance, int distanceCheckValue, int chipsOnLowLevel, int chipsOnTopLevel) {
//     if (sensorDistance <= distanceCheckValue) {
//         FastLED.clear();
//         for (int i = 0; chipsOnLowLevel; i++) {
//             leds[i] = CRGB(LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR);
//         }
//         for (int i = 0; chipsOnTopLevel; i++) {
//             leds[i] = CRGB(LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR, LED_BRIGHTNESS_SENSOR);
//         }
//         FastLED.show();
//     } else {
//         FastLED.clear();
//     }
// }

void disableLights() {
        FastLED.clear();
}