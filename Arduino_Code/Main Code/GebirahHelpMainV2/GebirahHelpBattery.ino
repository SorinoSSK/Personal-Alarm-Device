static void initBattery()
{
    chargeState = isCharging();
    pinMode(VBAT_ENABLE, OUTPUT);
    pinMode(BAT_CHARGE_STATE, INPUT);
    digitalWrite(BAT_HIGH_CHARGE, HIGH); // charge with 50mA
}

static bool isCharging()
{
    return digitalRead(BAT_CHARGE_STATE) == 0;
}

static void CheckBatteryChargingState()
{
    bool newChargeState = isCharging();
    if (newChargeState != chargeState)
    {
        chargeState = newChargeState;
        if (newChargeState)
        {
            Serial.println("Charging Battery");
        }
        else
        {
            Serial.println("Battery Stopped Charging");
        }
    }
}

static float getBatteryVoltage()
{
    digitalWrite(VBAT_ENABLE, LOW);

    uint32_t adcCount = analogRead(PIN_VBAT);
    float adcVoltage = adcCount * ADC_Vref/1024;
    float vBat = adcVoltage * (1510.0 / 510.0);

    digitalWrite(VBAT_ENABLE, HIGH);

    return vBat;
}

static float readBattery()
{
    long newBatteryReadTime = millis();
    if (newBatteryReadTime - batteryReadTime >= 1000)
    {
        Serial.print(analogRead(PIN_VBAT));
        Serial.println(getBatteryVoltage());
        batteryReadTime = millis();
    }
}