static void soundBuzzer()
{
    if (FirstBtnStatus || SeconBtnStatus || IMUFallDetected)
    {
        // long currentTimer = millis();
        // if (currentTimer - beepTimer > 500)
        // {
        //     BuzzerState = !BuzzerState;
        //     beepTimer = millis();
        // }
        // if (BuzzerState)
        // {
        //     digitalWrite(BuzzerPin, HIGH);
        // }
        // else
        // {
        //     digitalWrite(BuzzerPin, LOW);
        // }
        digitalWrite(BuzzerPin, HIGH);
    }
    else
    {
        digitalWrite(BuzzerPin, LOW);
    }
}