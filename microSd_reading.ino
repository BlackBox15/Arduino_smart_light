void readSdCard() {
    File openedFile;
    String rawText;
    char temporaryChar;
    bool enableGetNextChar = false;

    if (SD.begin(SD_CHIP_SELECT_PIN))
    {
        openedFile = SD.open(F("settings.txt"), FILE_READ);

        if (openedFile)
        {
            while (openedFile.available())
            {
                temporaryChar = (char)openedFile.read();

                if (temporaryChar == '#')
                {
                    enableGetNextChar = false;
                } else if (enableGetNextChar == false && temporaryChar == '\n')
                {
                    enableGetNextChar = true;
                    continue;
                }

                if (enableGetNextChar)
                {
                    rawText += temporaryChar;
                }
            }
            openedFile.close();    // закрываем файл
#ifdef DEBUG
            Serial.println(F("rawText: "));
            Serial.println(rawText);
#endif
            parseSettingsFile(rawText, timeParams);               // парсим настройки времени из прочитанного текста с SD-карты
            Ds1302::DateTime dt = fillDateTime(timeParams);       // заполняем структуру времени
            EEPROM.put(EEPROM_TIMEPARAMS_ADDRESS, timeParams);    // пишем в EEPROM настройки времени
            rtc.setDateTime(&dt);                                 // передаём на часы новое текущее время, прочитанное с SD-карты
        } else
        {
#ifdef DEBUG
            Serial.println(F("Can't open settings file."));
#endif
        }
    }
}

void parseSettingsFile(String inputString, int resultArray[]) {
    resultArray[0] = inputString.substring(0, 2).toInt();             // hour, current time
    resultArray[1] = inputString.substring(3, 5).toInt();             // minute, current time
    resultArray[2] = inputString.substring(6, 8).toInt();             // day, current time
    resultArray[3] = inputString.substring(9, 11).toInt();            // month, current time
    resultArray[4] = inputString.substring(12, 16).toInt() % 2000;    // year, current time
    resultArray[5] = inputString.substring(18, 20).toInt();           // hour, when lights on by sensor
    resultArray[6] = inputString.substring(21, 23).toInt();           // minute, when linght on by sensor
    resultArray[7] = inputString.substring(25, 27).toInt();           // hour, stand by lights
    resultArray[8] = inputString.substring(28, 30).toInt();           // minute, stand by lights
    resultArray[9] = inputString.substring(32, 34).toInt();           // hour, lights off
    resultArray[10] = inputString.substring(35, 37).toInt();          // minute, lights off
}

Ds1302::DateTime fillDateTime(int timeSettings[]) {
    Ds1302::DateTime dt;
    dt.year = timeSettings[4];
    dt.month = timeSettings[3];
    dt.day = timeSettings[2];
    dt.hour = timeSettings[0];
    dt.minute = timeSettings[1];
    dt.second = 0;
    return dt;
}