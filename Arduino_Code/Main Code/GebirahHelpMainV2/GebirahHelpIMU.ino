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
    // IMUVal[IMUBufferSize][0] = myIMU.readFloatAccelX();
    // IMUVal[IMUBufferSize][1] = myIMU.readFloatAccelY();
    // IMUVal[IMUBufferSize][2] = myIMU.readFloatAccelZ();
    // IMUVal[IMUBufferSize][3] = myIMU.readFloatGyroX();
    // IMUVal[IMUBufferSize][4] = myIMU.readFloatGyroY();
    // IMUVal[IMUBufferSize][5] = myIMU.readFloatGyroZ();
    float AcceNorm = sqrt(pow(myIMU.readFloatAccelX(),2)+pow(myIMU.readFloatAccelY(),2)+pow(myIMU.readFloatAccelZ(),2));
    float GyroNorm = sqrt(pow(myIMU.readFloatGyroX(),2)+pow(myIMU.readFloatGyroY(),2)+pow(myIMU.readFloatGyroZ(),2));
    FallDetectionType1(&AcceNorm, &GyroNorm);
    // Serial.print(AcceNorm);
    // Serial.print(" - ");
    // Serial.println(GyroNorm);
}

static void FallDetectionType1(float* AccelNorm, float* GyroNorm)
{
    // Activate FirstBtnStatus
    IMUDiffVal[0] = IMUDiffVal[1];
    IMUDiffVal[1] = *AccelNorm;
    float differential = (IMUDiffVal[1] - IMUDiffVal[0]) / 2;
    if (differential >= lowerAccelTresh)
    {
        if (Debug_Status != 0)
        {
            Serial.println("Falling Detected: " + String(*AccelNorm) + " - " + String(differential) + " - " + String(*GyroNorm));
        }
        // bluetoothModified = true;
        Fall_Detected += 1 ;
        FirstBtnStatus = true;
    }
}