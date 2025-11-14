#include <NewPing.h>
#include <SD.h>
#include <Arduino.h>
#include <Ds1302.h>
#include <EEPROM.h>
#include <FastLED.h>

#define DEBUG                           1
#define ULTRASONIC_1_TRIGGER_PIN        2
#define ULTRASONIC_1_ECHO_PIN           3
#define ULTRASONIC_2_TRIGGER_PIN        4
#define ULTRASONIC_2_ECHO_PIN           9
#define ULTRASONIC_CHECK_PERIOD_MS      2500    // период опроса УЗ-датчика, мс
#define ULTRASONIC_SWITCH_DISTANCE      10      // дистанция срабатывания УЗ-датчика, см
NewPing UltraSonicSensor1(ULTRASONIC_1_TRIGGER_PIN, ULTRASONIC_1_ECHO_PIN);
NewPing UltraSonicSensor2(ULTRASONIC_2_TRIGGER_PIN, ULTRASONIC_2_ECHO_PIN);

#define BUTTON_PIN                      A0

#define TIME_CHECK_PERIOD_MS            3000
#define TEST_LED                        8
#define SD_CHIP_SELECT_PIN              10

unsigned char timeParams[11];   

#define PIN_RST                         7       // Пин для управления
#define PIN_DAT                         6       // Пин для управления
#define PIN_CLK                         5       // Пин для управления
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);

#define LED_PIN                         A1      // Пин управления лентой
#define LED_NUM                         181     // количество чипов (для WS2811 каждый чип управляет 3мя диодами)
#define LED_BRIGHTNESS_STBY             15      // яркость диодов во включенном дежурном режиме
#define LED_BRIGHTNESS_SENSOR           15      // яркость диодов во включенном состоянии по сенсору
#define LED_CHIPS_ON_FLOOR              17      // число чипов на ступень

#define BUTTON_FILTER_TIME_MS           200     // настройки фильтрации дребезга кнопки, мс
CRGB leds[LED_NUM];
bool lightSwitch;
int initFlag;

// =================================================================//
// 																	//
//                      functions declaration                       //
// 																	//
// =================================================================//
int ReadInitFlag();
bool checkDistance(unsigned int, unsigned int, unsigned int);
bool fallTrigger(unsigned int, unsigned int);
// =================================================================//
// 																	//
//								setup								//
// 																	//
// =================================================================//
void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(TEST_LED, OUTPUT);

    // поднятие последовательного порта
    Serial.begin(9600);
    while (!Serial);
    
    Ds1302::DateTime initTime;

    rtc.init();

    // обнуление настроечных параметров на старте МК
    for (int i = 0; i < 11; i++) {
        timeParams[i] = 0;
    }

    // rtc.getDateTime(&initTime);
    timeParams[0] = 13;
    timeParams[1] = 03;
    timeParams[2] = 10;
    timeParams[3] = 11;
    timeParams[4] = 25;

    EEPROM.put(0, timeParams);

    EEPROM.get(0, timeParams);

    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_NUM);
    FastLED.setBrightness(LED_BRIGHTNESS_STBY);
    
    // FastLED.show();
    // printTimeSettings();
    #ifdef DEBUG
        // вывод настроечных параметров в момент запуска МК
        Serial.println();
        Serial.println(F("timeParams on start MCU:"));
        for (int i = 0; i < 11; i++) {
            Serial.print("[");
            Serial.print(i);
            Serial.print("] = ");
            Serial.println(timeParams[i]);
        }
    #endif
}
// =================================================================//
// 																	//
//								loop								//
// 																	//
// =================================================================//
void loop() {
    static Ds1302::DateTime now;
    unsigned long currentMillis = millis();
    static unsigned long prevTimeCheck;
    static unsigned long prevDistanceCheck;
    static bool isFiltrationActive;
    static unsigned long buttonPressTimestamp;
    static unsigned int prevDistSensor1;
    static unsigned int prevDistSensor2;
    unsigned int currDistSensor1;
    unsigned int currDistSensor2;
    static String currentState = F("Init");
    static bool isLedSwitchedOff;
    static bool isDistanceShort;
    String rawTextFromSd = F("");

    // Срабатывание кнопки + снятый флаг фильтрации дребезга
    if (!digitalRead(BUTTON_PIN) && !isFiltrationActive) {
        #ifdef DEBUG
            Serial.println(F("Button pressed..."));
        #endif
        buttonPressTimestamp = millis();                    // фиксация момента срабатывания кнопки
        isFiltrationActive = !isFiltrationActive;           // активация фильтрации дребезга кнопки
        readSdCard();                                       // необходимая работа по кнопке (чтение SD-карты)
    }

    // Снимаем флаг фильтрации дребезга по истечении времени уставки при условии, что флаг был поднят ранее
    if (isFiltrationActive && (currentMillis - buttonPressTimestamp >= BUTTON_FILTER_TIME_MS)) {
        isFiltrationActive = false;
    }

    // Периодическое обновление текущего времени
    if (currentMillis - prevTimeCheck >= TIME_CHECK_PERIOD_MS) {
        rtc.getDateTime(&now);
        prevTimeCheck = currentMillis;
        printTimeFromRtc(now);

        #ifdef DEBUG
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
        #endif
    }

    /*
        Описание уставок:

            0		timeSettings[9]		timeSettings[5]		timeSettings[7]		24
            |--------|-------------------|-------------------|------------------|
                stby		off					sensor			stby
    */

    // Период времени работы освещения по УЗ датчикам / дежурка
    if (now.hour >= timeParams[5] && now.hour <= timeParams[7]) {
        // Замер дистанции УЗ датчиками с заданным периодом
        if (currentMillis - prevDistanceCheck >= ULTRASONIC_CHECK_PERIOD_MS) {
            currDistSensor1 = UltraSonicSensor1.ping_cm();
            currDistSensor2 = UltraSonicSensor2.ping_cm();

            if (checkDistance(currDistSensor1, currDistSensor2, ULTRASONIC_SWITCH_DISTANCE) 
                && (fallTrigger(prevDistSensor1, currDistSensor1) 
                    || fallTrigger(prevDistSensor2, currDistSensor2))) {
                if (isLedSwitchedOff) {
                    isLedSwitchedOff = false;
                }

            }
            

            prevDistSensor1 = currDistSensor1;
            prevDistSensor2 = currDistSensor2;




            // isDistanceShort = checkDistance(distanceFromSensor1, distanceFromSensor2, ULTRASONIC_SWITCH_DISTANCE);
            
            // // Пересечение уставки по любому из 2-х УЗ датчиков и свет до этого момента был выключен
            // if (isDistanceShort && isLedSwitchedOff)) {
            //     isLedSwitchedOff = false;                       // состояние переменной соответствует ВКЛЮЧЕННОМУ свету
            //     #ifdef DEBUG
            //         currentState = F("Sensor On");
            //     #endif
            //         digitalWrite(TEST_LED, HIGH);
            //         fill_solid(leds, LED_NUM, CRGB::Gray100);   // включаем свет
            //         FastLED.show();


            //     // if (!isLedSwitchedOff) {
                    
            //     //     isLedSwitchedOff = !isLedSwitchedOff;
            //     //     digitalWrite(TEST_LED, HIGH);
            //     //     fill_solid(leds, LED_NUM, CRGB::Gray100);
            //     //     FastLED.show();
            //     // } 
            //     // else {
            //     //     #ifdef DEBUG
            //     //         currentState = F("Sensor Off");
            //     //     #endif
            //     //     isLedSwitchedOff = !isLedSwitchedOff;
            //     //     digitalWrite(TEST_LED, LOW);
            //     //     fill_solid(leds, LED_NUM, CRGB::Gray0);                                     // заливаем всю ленту чёрным (отключаем)
            //     //     for (unsigned char i = 0; i < LED_CHIPS_ON_FLOOR; i++) {                    // заливаем 1-ю ступень белым
            //     //         leds[i] = CRGB::Gray100;
            //     //     }
            //     //     for (unsigned char i = LED_NUM; i > (LED_NUM - LED_CHIPS_ON_FLOOR); i--) {  // заливаем последнюю ступень белым
            //     //         leds[i] = CRGB::Gray100;
            //     //     }
            //     //     FastLED.show();
            //     // }
            // }
        }
        
    // Период времени работы чисто дежурки
    } else if ((now.hour > timeParams[7] && now.hour <= 23) || (now.hour >= 0 && now.hour < timeParams[9])) {
        #ifdef DEBUG
            currentState = F("Stby");
        #endif
        leds[0] = CRGB::Gray25;
        leds[LED_NUM-1] = CRGB::Gray25;
        FastLED.show();
        digitalWrite(TEST_LED, HIGH);
    // Если ни в один диапазон устовок не проходим - время полного отключения освещения (день)
    } else {
        #ifdef DEBUG
            currentState = F("Daylight Off");
        #endif
        digitalWrite(TEST_LED, LOW);
        fill_solid(leds, LED_NUM, CRGB::Gray0);
        FastLED.show();        
    }
}

bool checkDistance(unsigned int sensor1, unsigned int sensor2, unsigned int distanceSetpoint) {
    return (sensor1 <= distanceSetpoint) || (sensor2 <= distanceSetpoint);
}

bool fallTrigger(unsigned int prevValue, unsigned int currValue) {
    return currValue < prevValue;
}