static void IMUInit()
{
    if (myIMU.begin() != 0)
    {
        if (Debug_Status != 0)
        {
            Serial.println("IMU failed to operate!");
        }
    }
    else
    {
        if (Debug_Status != 0)
        {
            Serial.println("IMU started and ready!");
        }
    }
}