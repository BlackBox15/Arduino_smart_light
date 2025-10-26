void printTimeFromRtc(Ds1302::DateTime timeData) {
    Serial.println(F("time from rtc:"));
    Serial.print(timeData.hour);
    Serial.print(F(":"));
    Serial.println(timeData.minute);
    Serial.print(timeData.day);
    Serial.print(F("-"));
    Serial.print(timeData.month);
    Serial.print(F("-"));
    Serial.println(timeData.year);
}