#include <NewPing.h>
#include <SD.h>
#include <Arduino.h>
#include <Ds1302.h>

#define ULTRASONIC_TRIGGER_PIN      2
#define ULTRASONIC_ECHO_PIN         3
#define ULTRASONIC_CHECK_PERIOD_MS  4000
#define ULTRASONIC_SWITCH_DISTANCE  10
#define TIME_CHECK_PERIOD_MS        1000
#define LED_STRIPE_DO               8
#define SD_CHIP_SELECT_PIN          10

NewPing UltraSonicSensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
unsigned long prevDistanceCheck;
unsigned long prevTimeCheck;
unsigned long distance;
bool ledStateLogic;

// Определение пинов для подключения DS1302
const int PIN_RST = 7;  // Пин для управления
const int PIN_DAT = 6;  // Пин для передачи данных
const int PIN_CLK = 5;  // Пин для синхронизации тактов
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);
int timeSettings[11];

// =================================================================//
// 																	//
//								setup								//
// 																	//
// =================================================================//
void setup() {
    pinMode(LED_STRIPE_DO, OUTPUT);
    File openedFile;
    String rawText;
    char temporaryChar;
    bool enableGetNextChar = false;
    String defaultInitTime = "Mon Aug 11 00:00:00 2025";

    Serial.begin(9600);
    while (!Serial);

    rtc.init();

    if (SD.begin(SD_CHIP_SELECT_PIN)) {
        openedFile = SD.open(F("settings.txt"), FILE_READ);

        if (openedFile) {
            while (openedFile.available()) {
                temporaryChar = (char)openedFile.read();
                            
                if (temporaryChar == '#') {
                    enableGetNextChar = false;
                } else if (enableGetNextChar == false && temporaryChar == '\n') {
                    enableGetNextChar = true;
                    continue;
                }

                if (enableGetNextChar) {
                    rawText += temporaryChar;
                }                
            }        
            openedFile.close();
            
            parseSettingsFile(rawText, timeSettings);

            Ds1302::DateTime dt = fillDateTime(timeSettings);

            rtc.setDateTime(&dt);
        }
    }
}
// =================================================================//
// 																	//
//								loop								//
// 																	//
// =================================================================//
void loop() {
    unsigned long currentMillis = millis();
    Ds1302::DateTime now;

    if (currentMillis - prevTimeCheck >= TIME_CHECK_PERIOD_MS) {
        rtc.getDateTime(&now);
        prevTimeCheck = currentMillis;
    }
	
    if (currentMillis - prevDistanceCheck >= ULTRASONIC_CHECK_PERIOD_MS) {
        distance = UltraSonicSensor.ping_cm();

        printTimeFromRtc(now);        

        if (now.hour >= timeSettings[9] || now.hour < timeSettings[5]) {
            digitalWrite(LED_STRIPE_DO, LOW);
        } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
            if (distance <= ULTRASONIC_SWITCH_DISTANCE) {
                if (digitalRead(LED_STRIPE_DO)) {
                    digitalWrite(LED_STRIPE_DO, LOW);
                } else {
                    digitalWrite(LED_STRIPE_DO, HIGH);
                }
            }
        } else if (now.hour >= timeSettings[7] || now.hour < timeSettings[9]) {
            digitalWrite(LED_STRIPE_DO, HIGH);
        }

        printDistance();

        prevDistanceCheck = currentMillis;
    }
}