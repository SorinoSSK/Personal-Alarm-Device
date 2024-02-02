static void bluetoothFunction() {
    // Check for modification of data
    bool bluetoothModified = false;
    // Call this every loop to publish bluetooth
    BLE.poll();
    // listen for Bluetooth® Low Energy peripherals to connect:
    BLEDevice central = BLE.central();
    // if a central is connected to peripheral:
    if (central)
    {
        // if it is bluetooth's first connection, start timer and wait for authentication
        if (!bluetoothConnected)
        {
            if (Debug_Status != 0)
            {
                // print the central's MAC address:
                Serial.print("Bluetooth connected to device ");
                Serial.println(central.address());
                Serial.println("Waiting for authentication...");
            }
            bluetoothConnected = true;
            BLETimer = millis();
        }
    }

    // while the central is still connected to peripheral:
    if (central.connected())
    {    
        // if bluetooth is authenticated, perform task
        if (bluetoothAuthenticated)
        {
            if (getDvStatus.written())
            {
                if (Debug_Status != 0)
                {
                    Serial.print("Received request for Device Status: ");
                }
                if(String(getDvStatus.value()) == "getIMUStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("IMU - ");
                        Serial.println(String(myIMU.begin() == 0));
                    }
                    getDvStatus.setValue(String(myIMU.begin() == 0));
                }
                else if (String(getDvStatus.value()) == "getLEDStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("LED - ");
                        Serial.println("verify light");
                    }
                    getDvStatus.setValue("1");
                }
                else if (String(getDvStatus.value()) == "getBuzzerStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("Buzzer - ");
                        Serial.println("verify sound");
                    }
                    getDvStatus.setValue("1");
                }
                else if (String(getDvStatus.value()) == "getMICStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("MIC - ");
                        Serial.println(String(Mic.begin()));
                    }
                    getDvStatus.setValue(String(Mic.begin()));
                }
                else if (String(getDvStatus.value()) == "getQSPIStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("QSPI - ");
                        Serial.println(String(QSPI_IsReady() == NRFX_SUCCESS));
                    }
                    getDvStatus.setValue(String(QSPI_IsReady() == NRFX_SUCCESS));
                }
                else
                {
                    if (Debug_Status != 0)
                    {
                        Serial.println("Invalid device.");
                    }
                    getDvStatus.setValue("Invalid device.");
                }
            }
            if (DeviceToken.written())
            {
                if (Debug_Status != 0)
                {
                    Serial.print("Received data for Device Token: ");
                }
                if (!TokenModifyToken)
                {
                    if (jsonData["DResetToken"] == String(DeviceToken.value()))
                    {
                        if (Debug_Status != 0)
                        {
                            Serial.println("Reset token verified.");
                        }
                        DeviceToken.setValue("Reset verified.");
                        TokenModifyToken = true;
                        BLETimer = millis();
                    }
                    else
                    {
                        if (Debug_Status != 0)
                        {
                            Serial.println("Invalid reset token.");
                        }
                        DeviceToken.setValue("Invalid reset token");
                    }
                }
                else
                {
                    if (String(DeviceToken.value()).length() == strlen(jsonData["DeviceToken"]))
                    {
                        jsonData["DeviceToken"] = DeviceToken.value();
                        bluetoothModified = true;
                        if (Debug_Status != 0)
                        {
                            Serial.println("Token modified.");
                        }
                        DeviceToken.setValue("Token modified.");
                        TokenModifyToken = false;
                    }
                    else
                    {
                        if (Debug_Status != 0)
                        {
                            Serial.println("Invalid token length.");
                        }
                        DeviceToken.setValue("Invalid token length.");
                    }
                }
            }
            if (getPDMSmple.written())
            {
                if (Debug_Status != 0)
                {
                    Serial.print("Received request for PDM record: ");
                }
                if (String(getPDMSmple.value()) == "getSamples")
                {
                    Serial.println("steaming samples.");
                    recording = 1;
                    record_ready = false;  
                    getPDMSmple.setValue("Streaming samples");
                }
                else if (String(getPDMSmple.value()) == "stopSamples")
                {
                    Serial.println("stop steaming.");
                    stop_record = true;  
                    getPDMSmple.setValue("Stop streaming");
                }
                else
                {
                    getPDMSmple.setValue("Invalid command");
                    Serial.println("Invalid command.");
                }
            }
            if (EmergencyNo.written())
            {
                if (Debug_Status != 0)
                {
                    Serial.print("Received Emergency No.: ");
                    Serial.println(String(EmergencyNo.value()));
                }
                DynamicJsonDocument EmergencyList(NoOfEmergencyContact);
                DeserializationError EmergencyListError = deserializeJson(EmergencyList, String(EmergencyNo.value()));
                if (EmergencyListError)
                {
                    if (Debug_Status != 0)
                    {
                        Serial.println("Invalid list format!");
                    }
                    EmergencyNo.setValue("Invalid list format!");
                }
                else
                {
                    jsonData["EmergencyNo"] = EmergencyList;
                    bluetoothModified = true;
                    if (Debug_Status != 0)
                    {
                        Serial.println("Emergency list set!");
                    }
                    EmergencyNo.setValue("Emergency list set!");
                }
            }
            // If any modification data is written, store to memory
            if (bluetoothModified)
            {
                storeJSONToMemory();
                bluetoothModified = false;
            }
            if (FirstBtnStatus)
            {
                FirstBtnStatus = false;
                BtnCodeSend.setValue("SeowAlert1");
            }
            else if (SeconBtnStatus)
            {
                SeconBtnStatus = false;
                BtnCodeSend.setValue("ChongAlert2");
            }
            if (!recording && record_ready)
            {
                PDMsMicRecs.writeValue(recording_buf, sizeof(recording_buf));
                if (stop_record)
                {
                    stop_record = false;
                }
                else
                {
                    recording = 1;
                }
                record_ready = false; 
            }
        }
        // if bluetooth is not authenticated, wait for authentication
        // disconnect bluetooth on timeout
        else
        {
            // disconnect bluetooth on timeout
            unsigned long BLETimerNow = millis();
            if (BLETimerNow - BLETimer >= Bluetooth_Time_Out)
            {
                if (Debug_Status != 0)
                {
                    Serial.println("Bluetooth authentication timeout.");
                }
                BLESAuthNum.setValue("Timeout");
                delay(100);
                central.disconnect();
            }
            // if authentication characteristic has value, verify authentication
            if (BLESAuthNum.written())
            {
                if (Debug_Status != 0)
                {
                    Serial.print("Received Bluetooth Authentication No.: ");
                    Serial.println(String(BLESAuthNum.value()));
                }
                if (jsonData["DeviceToken"] == String(BLESAuthNum.value()))
                {
                    bluetoothAuthenticated = true;
                    BLESAuthNum.setValue("Bluetooth Successful.");
                    if (Debug_Status != 0)
                    {
                        Serial.println("Bluetooth Authentication Successful.");
                    }
                }
                else
                {
                    BLESAuthNum.setValue("Bluetooth Failed.");
                    if (Debug_Status != 0)
                    {
                        Serial.println("Bluetooth Authentication Failed.");
                    }
                }
            }
        }
        resetTokenTimer();
            
    }
    else
    {
        // if there is a bluetooth connection and is disconnected, reset status
        if (bluetoothConnected)
        {
            bluetoothConnected = false;
            bluetoothAuthenticated = false;
            recording = 0;
            record_ready = false; 
            if (Debug_Status != 0)
            {
                Serial.print("Bluetooth disconnected from device ");
                Serial.println(central.address());
            }
        }
    }
}

static void resetTokenTimer()
{
    if (TokenModifyToken) 
    {
        unsigned long TokenTimerNow = millis();
        if (TokenTimerNow - BLETimer >= Modify_Token_Time_Out)
        {
            if (Debug_Status != 0)
            {
                Serial.println("Modify token timeout.");
            }
            TokenModifyToken = false;
            DeviceToken.setValue("Timeout");
        }
    }
}
