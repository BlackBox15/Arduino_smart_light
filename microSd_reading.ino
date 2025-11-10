void readSdCard() {
    File openedFile;
    String rawText;
    char temporaryChar;
    bool enableGetNextChar = false;

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

            // Serial.println(F("rawtext:"));
            // Serial.println(rawText);
            
            parseSettingsFile(rawText, timeParams);

            Ds1302::DateTime dt = fillDateTime(timeParams);
            // EEPROM.put(0, timeParams);

            rtc.setDateTime(&dt);
        } else {
            Serial.println(F("Can't open settings file."));
        }
    }
}