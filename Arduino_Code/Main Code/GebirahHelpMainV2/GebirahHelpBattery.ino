static void initBattery()
{
    chargeState = isCharging();
    pinMode(VBAT_ENABLE, OUTPUT);
    pinMode(BAT_CHARGE_STATE, INPUT);
    if (fastCharging)
    {
        digitalWrite(BAT_HIGH_CHARGE, LOW); // charge with 50mA
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
            Serial.println("Charging Battery");
        }
        else
        {
            Serial.println("Battery Stopped Charging");
        }
    }
}

// DO NOT use together with CheckBatteryChargingState()
static void CheckBatteryChargingStateBluetooth()
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
            BatCharStat.setValue("Charging");
        }
        else
        {
            if (Debug_Status != 0)
            {
                Serial.println("Battery Stopped Charging");
            }
            BatCharStat.setValue("Discharging");
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
        // if (Debug_Status != 0)
        // {
        //     Serial.println(getBatteryVoltage());
        // }
        batteryReadTime = millis();
        if (returnPercentage)
        {
            return map(getBatteryVoltage(), 3.7, 4.2, 0, 100);
        }
        else
        {
            return getBatteryVoltage();
        }
    }
}

// DO NOT use together with readBattery()
static void readBatteryBluetooth()
{
    long newBatteryReadTime = millis();
    if (newBatteryReadTime - batteryReadTime >= 1000)
    {
        // if (Debug_Status != 0)
        // {
        //     Serial.println(getBatteryVoltage());
        // }
        batteryReadTime = millis();
        float batteryVoltage = getBatteryVoltage();
        if (returnPercentage)
        {
            // Serial.print(batteryVoltage);
            // Serial.print(" ");
            // Serial.print(SOCKalmanFilter(batteryVoltage));
            // Serial.print(" ");
            // Serial.println(map(round(batteryLimit(batteryVoltage)*10.0), 37, 42, 0, 100));
            BatteryStat.setValue(String(map(round(batteryLimit(SOCKalmanFilter(batteryVoltage))*100.0), 370, 420, 0, 100)));
        }
        else
        {
            // Serial.println(String(batteryVoltage));
            BatteryStat.setValue(String(batteryVoltage));
        }
    }
}

static float batteryLimit(float value) 
{
    if (value > 4.2)
    {
        return 4.2;
    }
    else if (value < 3.7)
    {
        return 3.7;
    }
    else
    {
        return value;
    }

}