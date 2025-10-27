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