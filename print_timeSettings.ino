void printTimeSettings() {
    Serial.println();
    Serial.println("timesettings:");
    for (int i = 0; i < 11; i++) {
        Serial.print("[");
        Serial.print(i);
        Serial.print("] = ");
        Serial.println(timeParams[i]);
    }
}