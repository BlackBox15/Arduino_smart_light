#include <EEPROM.h>
#include <FastLED.h>
#include <Wire.h>
#include <iarduino_RTC.h>
#include <Ultrasonic.h>

#define LED_PIN 7     // пин
#define LED_NUM 15     // количество светодиодов

Ultrasonic ultrasonic(2, 3);
iarduino_RTC time(RTC_DS1302, 10, 12, 11);
CRGB leds[LED_NUM];
bool lightSwitch;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, LED_NUM);
    // FastLED.setBrightness(10);

    // FastLED.show();

    time.begin();
    // time.settime(0, 47, 11, 11, 7, 25, 5);
}

void loop() {
    if (ultrasonic.read() < 10) {
        lightSwitch = !lightSwitch;
    }

    if (lightSwitch) {
        for (int i = 0; i < LED_NUM; i++) {
            leds[i].setRGB(3, 3, 3);   // RGB, 0-255
        }
    } else {
        for (int i = 0; i < LED_NUM; i++) {
            leds[i].setRGB(0, 0, 0);   // RGB, 0-255
        }
    }
    

    Serial.print("Distance in CM: ");
    Serial.println(ultrasonic.read());
    Serial.println(time.gettime("d M Y, D"));
    Serial.println(time.gettime("H:i:s"));

    FastLED.show();
    delay(3000);
}
