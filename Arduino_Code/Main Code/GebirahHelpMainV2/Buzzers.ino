static void soundBuzzer()
{
    uint8_t period = 500;
    if (FirstBtnStatus || SeconBtnStatus || IMUFallDetected)
    {
        long currentTimer = millis();
        if (currentTimer - beepTimer > 500)
        {
            BuzzerState = !BuzzerState;
            beepTimer = millis();
        }
        if (BuzzerState)
        {
            digitalWrite(BuzzerPin, HIGH);
            // tone(BuzzerPin, 2000);
            // BuzzerStart();
        }
        else
        {
            digitalWrite(BuzzerPin, LOW);
            // noTone(BuzzerPin);
            // BuzzerStop();
        }
    }
    else
    {
        digitalWrite(BuzzerPin, LOW);
        // noTone(BuzzerPin);
        // BuzzerStop();
    }
}

static void initBuzzer()
{
    pinMode(BuzzerPin, OUTPUT);
}