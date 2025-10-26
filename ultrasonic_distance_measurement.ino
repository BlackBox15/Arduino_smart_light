// void measureDistance() {
//     ultrasonicDistance = UltraSonicSensor.ping_cm();
//         rtc.getDateTime(&now);

//         Serial.println(F("time from rtc:"));
//         Serial.print(now.hour);
//         Serial.print(F(":"));
//         Serial.println(now.minute);
//         Serial.print(now.day);
//         Serial.print(F("-"));
//         Serial.print(now.month);
//         Serial.print(F("-"));
//         Serial.println(now.year);

//         if (now.hour >= timeSettings[9] || now.hour < timeSettings[5]) {
//             digitalWrite(LED_STRIPE_DO, LOW);
//         } else if (now.hour >= timeSettings[5] && now.hour < timeSettings[7]) {
//             if (ultrasonicDistance <= ULTRASONIC_SWITCH_DISTANCE) {
//                 if (digitalRead(LED_STRIPE_DO)) {
//                     digitalWrite(LED_STRIPE_DO, LOW);
//                 } else {
//                     digitalWrite(LED_STRIPE_DO, HIGH);
//                 }
//             }
//         } else if (now.hour >= timeSettings[7] || now.hour < timeSettings[9]) {
//             digitalWrite(LED_STRIPE_DO, HIGH);
//         }

//         Serial.print("distance: ");
//         Serial.print(ultrasonicDistance);
//         Serial.println(" cm");

//         // for (int i = 0; i < 11; i++) {
//         //     Serial.print("timesettings[");
//         //     Serial.print(i);
//         //     Serial.print("] = ");
//         //     Serial.println(timeSettings[i]);
//         // }
// }