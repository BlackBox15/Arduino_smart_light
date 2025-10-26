void readSdCard() {
    File openedFile;
    String rawText;
    char temporaryChar;
    bool enableGetNextChar = false;
    // String defaultInitTime = "Mon Aug 11 00:00:00 2025";


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
            EEPROM.put(0, timeSettings);

            rtc.setDateTime(&dt);
        }
    }
}