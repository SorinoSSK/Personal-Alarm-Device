static double SOCKalmanFilter(float VoltVal)
{
    // float estBatteryVal     = 0.0;   // x - Initial Guess
    // float estBatteryP       = 1.0;   // P
    // float estBatteryF       = 1.0;   // F - Only 1 Sensor
    // float estBatteryH       = 1.0;   // H - Only 1 Measurement Value
    // float estBatteryKal     = 0.0;   // K

    // float estBatteryQ       = 1.0;   // Q - Near Calculated Value?
    // float estBatteryR       = 1.0;   // R - Near Computation
    
    // x_-
    estBatteryVal = estBatteryF*estBatteryVal;
    // P_-
    estBatteryP = estBatteryF*estBatteryP*estBatteryF + estBatteryQ;
    // K
    estBatteryKal = estBatteryP*estBatteryH/(estBatteryH*estBatteryP*estBatteryH + estBatteryR);
    // x_+
    estBatteryVal = estBatteryVal + estBatteryKal*(VoltVal - estBatteryH*estBatteryVal);
    // P_+
    estBatteryP = (1 - estBatteryKal*estBatteryH)*estBatteryP;

    return estBatteryVal;
}