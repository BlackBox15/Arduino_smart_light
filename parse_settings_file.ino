void parseSettingsFile(String inputString, unsigned char resultArray[]) {
    resultArray[0] = inputString.substring(0, 2).toInt();               // hour, current time
    resultArray[1] = inputString.substring(3, 5).toInt();               // minute, current time
    resultArray[2] = inputString.substring(6, 8).toInt();               // day, current time
    resultArray[3] = inputString.substring(9, 11).toInt();              // month, current time
    resultArray[4] = inputString.substring(12, 16).toInt() % 2000;      // year, current time

    resultArray[5] = inputString.substring(18, 20).toInt();             // hour, when lights on by sensor
    resultArray[6] = inputString.substring(21, 23).toInt();             // minute, when linght on by sensor
            
    resultArray[7] = inputString.substring(25, 27).toInt();             // hour, stand by lights
    resultArray[8] = inputString.substring(28, 30).toInt();             // minute, stand by lights
        
    resultArray[9] = inputString.substring(32, 34).toInt();             // hour, lights off
    resultArray[10] = inputString.substring(35, 37).toInt();            // minute, lights off

    /*
        for debug only
    */
    
    // Serial.println(F("parsing params:"));
    // for (int i = 0; i < 11; ++i) {
    //     Serial.print(F("timeSettings["));
    //     Serial.print(i);
    //     Serial.print(F("] = "));
    //     Serial.println(resultArray[i]);
    // }
}
