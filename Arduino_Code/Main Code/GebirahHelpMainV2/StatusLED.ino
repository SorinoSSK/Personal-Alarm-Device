static void initLED()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
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
}

static void resetStatus()
{
    if (LEDToReset)
    {
        long CurrentLEDTimer = millis();
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