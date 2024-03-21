static void debounce(uint8_t pinNo, bool* status, bool* firstStatus, bool* readyStatus, unsigned long* btnTimerActivate)
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
            unsigned long btnTimerCurrent = millis();
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

static void debounceLatch(uint8_t pinNo, bool* status, bool* firstStatus, bool* secondStatus, unsigned long* btnTimerActivate, bool* btnStatusSent)
{
    if (digitalRead(pinNo) && !*status)
    {    
        if (!*firstStatus)
        {
            *firstStatus = true;
            *secondStatus = false;
            *btnTimerActivate = millis();
        }
        else
        {
            unsigned long btnTimerCurrent = millis();
            // 200ms for human reaction time
            if (btnTimerCurrent - *btnTimerActivate > 100)
            {
                *firstStatus = false;
                *status = true;
                IMUFallDetected = false;
                IMUFallDetectedSent = false;
            }
        }
    }
    else if (!digitalRead(pinNo) && *status)
    {
        if (!*secondStatus)
        {
            *secondStatus = true;
            *firstStatus = false;
            *btnTimerActivate = millis();
        }
        else
        {
            unsigned long btnTimerCurrent = millis();
            // 200ms for human reaction time
            if (btnTimerCurrent - *btnTimerActivate > 100)
            {
                *secondStatus = false;
                *status = false;
                *btnStatusSent = false;
            }
        }
    }
}