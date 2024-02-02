static void debounce(uint8_t pinNo, bool* status, bool* firstStatus, bool* readyStatus, long* btnTimerActivate)
{
    if (digitalRead(pinNo))
    {    
        if (!*firstStatus)
        {
            *firstStatus = true;
            *btnTimerActivate = millis();
        }
        else
        {
            long btnTimerCurrent = millis();
            // 200ms for human reaction time
            if (btnTimerCurrent - *btnTimerActivate > 100)
            {
                *readyStatus = true;
            }
        }
    }
    else
    {
        if (*readyStatus)
        {
            *firstStatus = false;
            *readyStatus = false;
            *status = true;
        }
    }
}