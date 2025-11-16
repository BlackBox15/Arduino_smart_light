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
#define ULTRASONIC_CHECK_PERIOD_MS      3000    // период опроса УЗ-датчика, мс
#define ULTRASONIC_SWITCH_DISTANCE      10      // дистанция срабатывания УЗ-датчика, см
NewPing UltraSonicSensor1(ULTRASONIC_1_TRIGGER_PIN, ULTRASONIC_1_ECHO_PIN);
NewPing UltraSonicSensor2(ULTRASONIC_2_TRIGGER_PIN, ULTRASONIC_2_ECHO_PIN);

#define BUTTON_PIN                      A0

#define TIME_CHECK_PERIOD_MS            1000
#define TEST_LED                        8
#define SD_CHIP_SELECT_PIN              10

int timeParams[11];   

#define PIN_RST                         7       // Пин для управления
#define PIN_DAT                         6       // Пин для управления
#define PIN_CLK                         5       // Пин для управления
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);

#define LED_PIN                         A1      // Пин управления лентой
#define LED_NUM                         15     // количество чипов (для WS2811 каждый чип управляет 3мя диодами)
#define LED_BRIGHTNESS_STBY             15      // яркость диодов во включенном дежурном режиме
#define LED_BRIGHTNESS_SENSOR           15      // яркость диодов во включенном состоянии по сенсору
#define LED_CHIPS_ON_FLOOR              4      // число чипов на ступень

#define BUTTON_FILTER_TIME_MS           200     // настройки фильтрации дребезга кнопки, мс
#define EEPROM_INIT_ADDRESS             0       // адрес флага инициализации в EEPROM
#define EEPROM_TIMEPARAMS_ADDRESS       1       // адрес расположения настроек времени в EEPROM
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
    pinMode(BUTTON_PIN, INPUT_PULLUP);          // настройка пина BUTTON_PIN в режиме дискретного входа с использованием внутреннего подтягивающего резистора
    pinMode(TEST_LED, OUTPUT);                  // пин TEST_LED в режиме дискретного выхода, на этом пине сидит контрольный светодиод

    Serial.begin(9600);                         // поднимаем последовательный порт
    while (!Serial);
    
    Ds1302::DateTime initTime;

    rtc.init();

    if (EEPROM.read(EEPROM_INIT_ADDRESS) == 1) {                 // проверка флага инициализации
        EEPROM.get(EEPROM_TIMEPARAMS_ADDRESS, timeParams);      // 1 - говорит о том, что контроллер был ранее инициализирован, и
                                                                // ожидается, что в области EEPROM находятся настройки
        rtc.getDateTime(&initTime);

        timeParams[0] = initTime.hour;   
        timeParams[1] = initTime.minute; 
        timeParams[2] = initTime.day;    
        timeParams[3] = initTime.month;  
        timeParams[4] = initTime.year;   

    } else {                                                    // отсутсвие 1 по адресу инициализации говорит о том, что контроллер необходимо инициализировать
        // необходимо выполнить первоначальную инициализацию и запись настроек времени в EEPROM
        timeParams[0] = 01;     // 01 час
        timeParams[1] = 01;     // 01 минута
        timeParams[2] = 01;     // 01 день
        timeParams[3] = 01;     // 01 месяц
        timeParams[4] = 2025;     // 2025 год

        EEPROM.put(EEPROM_TIMEPARAMS_ADDRESS, timeParams);
        EEPROM.write(EEPROM_INIT_ADDRESS, 1);                   // по завершению инициализации запись флага по специальному адресу
    }

    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_NUM);
    FastLED.setBrightness(LED_BRIGHTNESS_STBY);
    
    // FastLED.show();
    // printTimeSettings();
    #ifdef DEBUG
        // вывод настроечных параметров в момент запуска МК
        Serial.println();
        Serial.println(F("timeParams on start MCU from variable:"));
        for (int i = 0; i < 11; i++) {
            Serial.print("array: [");
            Serial.print(i);
            Serial.print("] = ");
            Serial.println(timeParams[i]);
        }
        Serial.println();
        Serial.println(F("timeParams on start MCU from EEPROM:"));
        for (int i = 0; i < 15; i++) {
            Serial.print("address: [");
            Serial.print(i);
            Serial.print("] = ");
            Serial.println(EEPROM.read(i), DEC);
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
    static unsigned long prevDistSensor1;
    static unsigned long prevDistSensor2;
    static unsigned long currDistSensor1;
    static unsigned long currDistSensor2;
    static String currentState = F("Init");
    static bool isLedSwitchedOff = true;
    static bool isDistanceShort;
    static bool isStbyAfterMidnight;
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

        #endif
    }

    /*
        Описание уставок:

            0		timeSettings[9]		timeSettings[5]		timeSettings[7]		24
            |--------|-------------------|-------------------|------------------|
                stby		off					sensor			stby
    */

    if ((timeParams[7] > timeParams[5]) && (timeParams[7] <= 23)) {
        isStbyAfterMidnight = false;
    } else {
        isStbyAfterMidnight = true;
    }

    if (now.hour >= timeParams[9] and now.hour <= timeParams[5]) {      // дневной режим, всё отключено
        #ifdef DEBUG
            currentState = F("Daylight Off");
        #endif
        digitalWrite(TEST_LED, LOW);
        fill_solid(leds, LED_NUM, CRGB::Gray0);
        FastLED.show();  
    } else if (isStbyAfterMidnight) {
        currentState = F("Stby after midnight");
        if (now.hour >= timeParams[7] && now.hour < timeParams[9]) {
            digitalWrite(TEST_LED, HIGH);
            fill_solid(leds, LED_NUM, CRGB::Gray10);
            FastLED.show();
        } else {
            currentState = F("Stby after midnight, sensor");
            bySensor(currentMillis);
        }
    } else if (!isStbyAfterMidnight) {
        currentState = F("Stby before midnight");
        if (now.hour > timeParams[5] && now.hour <= 23) {
            digitalWrite(TEST_LED, HIGH);
            fill_solid(leds, LED_NUM, CRGB::Gray10);
            FastLED.show();
        } else {
            currentState = F("Stby before midnight, sensor");
            bySensor(currentMillis);
        }
    } 
}

void bySensor(unsigned long currentMillis) {
    static unsigned long prevTimeCheck;
    static unsigned long prevDistanceCheck;
    static bool isFiltrationActive;
    static unsigned long buttonPressTimestamp;
    static unsigned long prevDistSensor1;
    static unsigned long prevDistSensor2;
    static unsigned long currDistSensor1;
    static unsigned long currDistSensor2;
    static String currentState = F("Init");
    static bool isLedSwitchedOff = true;
    static bool isDistanceShort;
    static bool isStbyAfterMidnight;
    if (currentMillis - prevDistanceCheck >= ULTRASONIC_CHECK_PERIOD_MS) {
        currDistSensor1 = UltraSonicSensor1.ping_cm();
        currDistSensor2 = UltraSonicSensor2.ping_cm();

        Serial.print(F("prevDist1: "));
        Serial.print(prevDistSensor1);
        Serial.print(F(" cm\t"));
        Serial.print(F("currDist1: "));
        Serial.print(currDistSensor1);
        Serial.println(F(" cm"));
        Serial.print(F("prevDist2: "));
        Serial.print(prevDistSensor2);
        Serial.print(F(" cm\t"));
        Serial.print(F("currDist2: "));
        Serial.print(currDistSensor2);
        Serial.println(F(" cm"));

        if ((currDistSensor1 < ULTRASONIC_SWITCH_DISTANCE && prevDistSensor1 > ULTRASONIC_SWITCH_DISTANCE) || 
                (currDistSensor2 < ULTRASONIC_SWITCH_DISTANCE && prevDistSensor2 > ULTRASONIC_SWITCH_DISTANCE)) {
            isLedSwitchedOff = !isLedSwitchedOff;

            Serial.print(F("*** isLedSwitchOff:\t"));
            Serial.print(isLedSwitchedOff);
            Serial.println();

        }    

        if (isLedSwitchedOff) {
            // дежурный свет
            digitalWrite(TEST_LED, HIGH);
            // fill_solid(leds, LED_NUM, CRGB::Gray10);
            fill_solid(leds, LED_NUM, CRGB::Gray0);

            FastLED.show();
        } else {
            // яркое включение верхней и нижней
            digitalWrite(TEST_LED, LOW);
            FastLED.clear();
            for (byte i = 0; i < LED_CHIPS_ON_FLOOR; i++) {                    // заливаем 1-ю ступень белым
                leds[i] = CRGB::Gray10;
            }
            for (byte i = LED_NUM; i > (LED_NUM - LED_CHIPS_ON_FLOOR); i--) {  // заливаем последнюю ступень белым
                leds[i] = CRGB::Gray10;
            }
            FastLED.show();
        }

        prevDistSensor1 = currDistSensor1;
        prevDistSensor2 = currDistSensor2;
        prevDistanceCheck = currentMillis;
    }
}