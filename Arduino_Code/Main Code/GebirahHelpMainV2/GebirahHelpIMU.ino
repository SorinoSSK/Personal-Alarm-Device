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
    if (jsonData["FallDetects"] == "0")
    {
        FallDetectionType1(&AcceNorm, &GyroNorm);
    }
    else if (jsonData["FallDetects"] == "1")
    {
        // FallDetectionType2(&AcceNorm, myIMU.readFloatAccelX(), myIMU.readFloatAccelY(), myIMU.readFloatAccelZ());
        FallDetectionType2(&AcceNorm);
    }
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
        Fall_Detected += 1;
        FirstBtnStatus = true;
    }
}

// static void FallDetectionType2(float* AccelNorm, float AccelX, float AccelY, float AccelZ)
// {
//     float h1 = 1/(1 + exp(-(-3 + 0.3239177*AccelX + 1.35821812*AccelY - 0.91075929*AccelZ - 1.32501476*(*AccelNorm))));
//     if (h1 >= 0.5)
//     {
//         if (Debug_Status != 0)
//         {
//             Serial.println("h1 Value: " + String(h1) + " - Falling Detected: " + String(*AccelNorm) + " - " + String(AccelX) + " - " + String(AccelY) + " - " + String(AccelZ));
//         }
//         Fall_Detected +=1;
//         FirstBtnStatus = true;
//     }
// }

static void FallDetectionType2(float* AccelNorm)
{
    float coeff[IMUBufferSize3+1] = {-0.00068809,  0.11390063, -0.26251334, -0.06533656, -0.20371173,
        -0.02432717, -0.19484031,  0.04825735,  0.06518361, -0.23732902,
         0.00741389};
    float zValue = coeff[0];
    for (int i = 1; i < IMUBufferSize3; i ++)
    {
        IMUBuffVal[i-1] = IMUBuffVal[i];
        zValue += coeff[i]*IMUBuffVal[i];
    }
    IMUBuffVal[IMUBufferSize3-1] = *AccelNorm;
    zValue += coeff[IMUBufferSize3-1]*IMUBuffVal[IMUBufferSize3-1];
    // float h1 = 1/(1 + exp(-(-3 + 0.3239177*AccelX + 1.35821812*AccelY - 0.91075929*AccelZ - 1.32501476*(*AccelNorm))));
    float h1 = 1/(1 + exp(-zValue));
    if (h1 >= 0.5)
    {
        if (Debug_Status != 0)
        {
            Serial.println("h1 Value: " + String(h1) + " - Falling Detected: " + String(*AccelNorm));
        }
        Fall_Detected +=1;
        FirstBtnStatus = true;
    }
}