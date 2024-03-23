static void initLED()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLE, OUTPUT);
    pinMode(LED_FIRST_BTN, OUTPUT);
    pinMode(LED_SECON_BTN, OUTPUT);
    resetLED();
}

static void runLED()
{
    LEDStatus();
    resetStatus();
}

static void LEDStatus()
{
    if (statusForLED != "")
    {
        resetLED();
        if (statusForLED == "completeSetup")
        {
            digitalWrite(LED_GREEN, LOW);
        }
        statusForLED = "";
        LEDToReset = true;
        LEDTimer = millis();
    }
    FirstBtnLEDStatus(&FirstBtnStatus, LED_FIRST_BTN);
    SeconBtnLEDStatus(&FirstBtnStatus, &SeconBtnStatus, LED_SECON_BTN);
    IMUFallLEDStatus(&FirstBtnStatus, &SeconBtnStatus, &IMUFallDetected, &IMUBlinkTimer, LED_FIRST_BTN, LED_SECON_BTN);
    BLELEDStatus(&bluetoothAuthenticated);
}

static void resetStatus()
{
    if (LEDToReset)
    {
        unsigned long CurrentLEDTimer = millis();
        if (CurrentLEDTimer - LEDTimer >= 1000)
        {
            resetLED();
            LEDToReset = false;
        }
    }
}

static void resetLED()
{
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
}

static void BLELEDStatus(bool *statusValue)
{
    if (chargeState)
    {
        blinkLED(&LED_BLE, &SideLEDState, &SideLEDTimer, 500);
    }
    else if (batteryLimit(BatteryVoltage) < 3.5)
    {
        blinkLED(&LED_BLE, &SideLEDState, &SideLEDTimer, 1000);
    }
    else if (*statusValue)
    {
        analogWrite(LED_BLE, 0);
    }
    else
    {
        analogWrite(LED_BLE, 255/4);
    }
}

static void FirstBtnLEDStatus(bool *statusValue, uint8_t pinNo)
{
    if (*statusValue)
    {
        analogWrite(pinNo, 255/4);
    }
    else
    {
        if (!IMUFallDetected)
        {
            analogWrite(pinNo, 0);
        }
    }
}

static void SeconBtnLEDStatus(bool *statusValue1, bool *statusValue2, uint8_t pinNo)
{
    if (*statusValue2 && !*statusValue1)
    {
        analogWrite(pinNo, 255/4);
    }
    else
    {
        if (!IMUFallDetected)
        {
            analogWrite(pinNo, 0);
        }
    }
}

static void IMUFallLEDStatus(bool *statusValue1, bool *statusValue2, bool *statusValue3, unsigned long* IMUBlinkTimer, uint8_t pinNo1, uint8_t pinNo2)
{
    unsigned long currTimer = millis();
    if (!*statusValue2 && !*statusValue1 && *statusValue3)
    {
        if (currTimer - *IMUBlinkTimer > IMUFallDetectedBlinkRate)
        {
            IMUFallDetectedLEDLight = !IMUFallDetectedLEDLight;
            *IMUBlinkTimer = millis();
        }
        if (IMUFallDetectedLEDLight)
        {
            analogWrite(pinNo1, 255/4);
            analogWrite(pinNo2, 255/4);
        }
        else
        {
            analogWrite(pinNo1, 0);
            analogWrite(pinNo2, 0);
        }
    }
}

static void blinkLED(uint8_t *pinNo, bool *status, unsigned long *timer, uint8_t blinkRate)
{
    unsigned long currTimer = millis();
    if (currTimer - *timer > blinkRate)
    {
        *status = !*status;
        *timer = millis();
    }
    if (*status)
    {
        analogWrite(*pinNo, 255/4);
    }
    else
    {
        analogWrite(*pinNo, 0);
    }
}