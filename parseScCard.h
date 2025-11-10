#include <SD.h>

void parseSdCard(SDLib::SDClass card) {
    File openedFile;
    String rawText;
    char temporaryChar;
    bool enableGetNextChar = false;

    if (card.begin(SD_CHIP_SELECT_PIN)) {
        openedFile = card.open(F("settings.txt"), FILE_READ);

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
        } else {
            rawText = "nothing";
        }
    }

    // return rawText;
}