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

static void IMUFunction()
{
    // Serial.print(" X1 = ");
    // Serial.print(myIMU.readFloatAccelX(), 4);
    // Serial.print(" ");
    // Serial.print(" X1 = ");
    // Serial.println(myIMU.readFloatGyroX(), 4);
    // Serial.print(" Y1 = ");
    // Serial.print(myIMU.readFloatAccelY(), 4);
    // Serial.print(" ");
    // Serial.print(" Y1 = ");
    // Serial.println(myIMU.readFloatGyroY(), 4);
    // Serial.print(" Z1 = ");
    // Serial.print(myIMU.readFloatAccelZ(), 4);
    // Serial.print(" ");
    // Serial.print(" Z1 = ");
    // Serial.println(myIMU.readFloatGyroZ(), 4);
    Serial.print("(");
    Serial.print(myIMU.readFloatAccelX(), 4);
    Serial.print(", ");
    Serial.print(myIMU.readFloatAccelY(), 4);
    Serial.print(", ");
    Serial.print(myIMU.readFloatAccelZ(), 4);
    Serial.print(") ");
    Serial.println(sqrt(pow(myIMU.readFloatAccelX(), 2) + pow(myIMU.readFloatAccelY(), 2) + pow(myIMU.readFloatAccelZ(), 2)));
}