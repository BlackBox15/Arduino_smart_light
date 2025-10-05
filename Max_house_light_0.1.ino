#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <Wire.h>
#include <iarduino_RTC.h>
#include <Ultrasonic.h>

#define LED_PIN 7                           // пин
#define LED_NUM 15                          // количество светодиодов
#define SD_PIN_CS 10

Ultrasonic ultrasonic(2, 3);                // Trig, Echo pins
unsigned long previousMillis = 0;
iarduino_RTC watch(RTC_DS1302, 4, 5, 6);     // для модуля DS1302 - RST, CLK, DAT, обмен по шине SPI
int timeSettings[11];

CRGB leds[LED_NUM];
bool lightSwitch;

// ==================================================================
// 
//                                  setup
// 
// ==================================================================
void setup() {
    File openedFile;
    String rawText;
    char temporaryChar;
    bool enableGetNextChar = false;
    char* defaultInitTime = "Mon Aug 11 00:00:00 2025";

    Serial.begin(9600);
    while (!Serial);

    SD.begin(10);
    openedFile = SD.open(F("settings.txt"), FILE_READ);

    if (openedFile) {
        Serial.println(F("Reading in progress..."));
        
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
        Serial.println(F("File closed"));
    } else {
        Serial.println(F("File isn't available"));
    }
    
    // Serial.println(F("Raw text:"));
    // Serial.println(rawText);
    
    parseSettingsFile(rawText, timeSettings);

    watch.begin();
    if (timeSettings[0] != "") {
        watch.settime(0, timeSettings[1], timeSettings[0], timeSettings[2], timeSettings[3], timeSettings[4], 1);
    } else {
        watch.settime(defaultInitTime);
    }

    // openedFile.close();

    // Serial.print("File is available: ");
    // Serial.println(fileIsAvailable);

    // Serial.print("File size: ");
    // Serial.println(fileSize);
    
    // Serial.print("File content: ");
    // Serial.println(fileContent);

    // if (settingsFile.available()) {
    //     Serial.println("Opening settings.txt...");

    //     while (settingsFile.available()) {
    //         Serial.println("file available...");
    //         tempString = settingsFile.readString();
            
    //         Serial.print("Was read: ");
    //         Serial.println(tempString);

    //         settingsArray[i] = tempString;
    //         i++;
    //         tempString = "";
    //         // if (tempString.charAt(0) != '#') {
    //         //     settingsArray[i++] = tempString;
    //         // }            
    //     }
    //     settingsFile.close();
    // } else {
    //     Serial.println("Error opening settings.txt");
    // }


    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_NUM);
    FastLED.setBrightness(10);
    FastLED.show();
}

// ==================================================================
// 
//                              loop
// 
// ==================================================================
void loop() {
    
   unsigned long currentMillis = millis();

   if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;

        Serial.println(watch.gettime("d-m-Y, H:i:s, D"));  // Выводим время.
   }

    
//     if (millis() % 1000 == 0) {
//       Serial.println(time.gettime("d-m-Y, H:i:s, D"));
//       delay(1);
//    }
    // if (ultrasonic.read() < 10) {
    //     lightSwitch = !lightSwitch;
    // }

    // if (lightSwitch) {
    //     for (int i = 0; i < LED_NUM; i++) {
    //         leds[i].setRGB(3, 3, 3);   // RGB, 0-255
    //     }
    // } else {
    //     for (int i = 0; i < LED_NUM; i++) {
    //         leds[i].setRGB(0, 0, 0);   // RGB, 0-255
    //     }
    // }
    

    // Serial.print("Distance in CM: ");
    // Serial.println(ultrasonic.read());
    // Serial.println(time.gettime("d M Y, D"));
    // Serial.println(time.gettime("H:i:s"));

    // FastLED.show();
    // delay(3000);
}

// ==================================================================
// 
//                      parseSettingsFile
// 
// ==================================================================
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

    Serial.println(F("parsing params:"));
    for (int i = 0; i < 11; ++i) {
        Serial.print(F("timeSettings["));
        Serial.print(i);
        Serial.print(F("] = "));
        Serial.println(resultArray[i]);
    }
}
