#include <NewPing.h>
#include <SD.h>
#include <Arduino.h>
#include <Ds1302.h>

#define ULTRASONIC_TRIGGER_PIN      2
#define ULTRASONIC_ECHO_PIN         3
#define ULTRASONIC_CHECK_PERIOD     4000
#define LED_STRIPE_DISTANCE         10

#define LED_STRIPE_DO               8
#define SD_CHIP_SELECT_PIN          10

NewPing UltraSonicSensor(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN);
unsigned long previousMillis;
unsigned long ultrasonicDistance;
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

            Ds1302::DateTime dt;
            dt.year = timeSettings[4];
            dt.month = timeSettings[3];
            dt.day = timeSettings[2];
            dt.hour = timeSettings[0];
            dt.minute = timeSettings[1];
            dt.second = 0;

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
	
    if (currentMillis - previousMillis >= ULTRASONIC_CHECK_PERIOD) {
        ultrasonicDistance = UltraSonicSensor.ping_cm();
        rtc.getDateTime(&now);

        Serial.println(F("time from rtc:"));
        Serial.print(now.hour);
        Serial.print(F(":"));
        Serial.println(now.minute);
        Serial.print(now.day);
        Serial.print(F("-"));
        Serial.print(now.month);
        Serial.print(F("-"));
        Serial.println(now.year);

        if (now.hour >= timeSettings[9] || now.hour < timeSettings[5]) {
            digitalWrite(LED_STRIPE_DO, LOW);
        } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
            if (ultrasonicDistance <= LED_STRIPE_DISTANCE) {
                if (digitalRead(LED_STRIPE_DO)) {
                    digitalWrite(LED_STRIPE_DO, LOW);
                } else {
                    digitalWrite(LED_STRIPE_DO, HIGH);
                }
            }
        } else if (now.hour >= timeSettings[7] || now.hour < timeSettings[9]) {
            digitalWrite(LED_STRIPE_DO, HIGH);
        }

        
        Serial.print("distance: ");
        Serial.print(ultrasonicDistance);
        Serial.println(" cm");

        // for (int i = 0; i < 11; i++) {
        //     Serial.print("timesettings[");
        //     Serial.print(i);
        //     Serial.print("] = ");
        //     Serial.println(timeSettings[i]);
        // }

        previousMillis = currentMillis;
    }


}
// =================================================================//
// 																	//
//					    parseSettingsFile							//
// 																	//
// =================================================================//
void parseSettingsFile(String inputString, int resultArray[]) {
    resultArray[0] = inputString.substring(0, 2).toInt();       // hour, current time
    resultArray[1] = inputString.substring(3, 5).toInt();       // minute, current time
    resultArray[2] = inputString.substring(6, 8).toInt();       // day, current time
    resultArray[3] = inputString.substring(9, 11).toInt();      // month, current time
    resultArray[4] = inputString.substring(12, 16).toInt() % 2000;     // year, current time

    resultArray[5] = inputString.substring(18, 20).toInt();     // hour, when lights on by sensor
    resultArray[6] = inputString.substring(21, 23).toInt();     // minute, when linght on by sensor
    
    resultArray[7] = inputString.substring(25, 27).toInt();     // hour, stand by lights
    resultArray[8] = inputString.substring(28, 30).toInt();     // minute, stand by lights

    resultArray[9] = inputString.substring(32, 34).toInt();     // hour, lights off
    resultArray[10] = inputString.substring(35, 37).toInt();    // minute, lights off

    // Serial.println(F("parsing params:"));
    // for (int i = 0; i < 11; ++i) {
    //     Serial.print(F("timeSettings["));
    //     Serial.print(i);
    //     Serial.print(F("] = "));
    //     Serial.println(resultArray[i]);
    // }
}
