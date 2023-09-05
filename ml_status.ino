void Status_ValueChangedStr(const char *descr, const char *value)
{
    Serial.printf("%s: %s\n", descr, value);
}

void Status_ValueChangedFloat(const char *group, const char *descr, float value)
{
    Serial.printf("%s - %s: %0.3f\n", group, descr, value);
}

void Status_ValueChangedFloat(const char *descr, float value)
{
    Serial.printf("%s: %0.3f\n", descr, value);
}
