static void initBattery()
{
    chargeState = isCharging();
    pinMode(VBAT_ENABLE, OUTPUT);
    pinMode(BAT_CHARGE_STATE, INPUT);
    // if (fastCharging)
    if (jsonData["FastChargng"] == "1")
    {
        digitalWrite(BAT_HIGH_CHARGE, LOW); // charge with 100mA
    }
    else
    {
        digitalWrite(BAT_HIGH_CHARGE, HIGH); // charge with 50mA
    }
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
            if (Debug_Status != 0)
            {
                Serial.println("Charging Battery");
            }
            if (bluetoothAuthenticated)
            {
                BatCharStat.setValue("CHARGING");
            }
        }
        else
        {
            if (Debug_Status != 0)
            {
                Serial.println("Battery Stopped Charging");
            }
            if (bluetoothAuthenticated)
            {
                BatCharStat.setValue("DISCHARGING");
            }
        }
    }
}

static float getBatteryVoltage()
{
    digitalWrite(VBAT_ENABLE, LOW);

    uint32_t adcCount = analogRead(PIN_VBAT);
    float adcVoltage = adcCount * ADC_Vref/1024;
    float vBat = adcVoltage * ((Voltage_Div_Num + Voltage_Div_Offset) / Voltage_Div_Den);
    // Serial.print(analogRead(PIN_VBAT));
    // Serial.print(" ");
    digitalWrite(VBAT_ENABLE, HIGH);

    return vBat;
}

static float readBattery()
{
    long newBatteryReadTime = millis();
    if (newBatteryReadTime - batteryReadTime >= 1000)
    {
        batteryReadTime = millis();
        SOCMeanFilter(getBatteryVoltage());
        BatteryReadingRdy = true;
    }
}

static void broadcastBatteryBluetooth()
{
    if (BatteryReadingRdy)
    {
        // if (returnPercentage)
        if (jsonData["BattVoltage"] == "0")
        {
            // Serial.print(batteryVoltage);
            // Serial.print(" ");
            // Serial.print(SOCKalmanFilter(batteryVoltage));
            // Serial.print(" ");
            // Serial.println(map(round(batteryLimit(batteryVoltage)*10.0), 37, 42, 0, 100));
            BatteryStat.setValue(String(map(round(batteryLimit(BatteryVoltage)*100.0), 335, 420, 0, 100)));
        }
        else
        {
            // Serial.println(String(batteryVoltage));
            BatteryStat.setValue(String(batteryLimit(BatteryVoltage)));
        }
        BatteryReadingRdy = false;
    }
}

static float batteryLimit(float value) 
{
    // if (BatterySoftLimit)
    if (jsonData["BattBounded"] == "1")
    {
        if (value > 4.2)
        {
            return 4.2;
        }
        else if (value < 3.35)
        {
            return 3.35;
        }
        else
        {
            return value;
        }
    }
    else
    {
        return value;
    }

}

static void runBattery()
{
    readBattery();
    CheckBatteryChargingState();
}