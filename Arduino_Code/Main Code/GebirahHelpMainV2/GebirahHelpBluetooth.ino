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
                    if (myIMU.begin() == 0)
                    {
                        getDvStatus.setValue(String("OK!"));
                    }
                    else
                    {
                        getDvStatus.setValue(String("FAILED!"));
                    }
                    
                }
                else if (String(getDvStatus.value()) == "getLEDStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("LED - ");
                        Serial.println("verify light");
                    }
                    getDvStatus.setValue("OK!");
                }
                else if (String(getDvStatus.value()) == "getBuzzerStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("Buzzer - ");
                        Serial.println("verify sound");
                    }
                    getDvStatus.setValue("OK!");
                }
                else if (String(getDvStatus.value()) == "getMICStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("MIC - ");
                        Serial.println(String(Mic.begin()));
                    }
                    if (Mic.begin())
                    {
                        getDvStatus.setValue(String("OK!"));
                    }
                    else
                    {
                        getDvStatus.setValue(String("FAILED!"));
                    }
                }
                else if (String(getDvStatus.value()) == "getQSPIStatus")
                {
                    if (Debug_Status != 0)
                    {
                        Serial.print("QSPI - ");
                        Serial.println(String(QSPI_IsReady() == NRFX_SUCCESS));
                    }
                    if (QSPI_IsReady() == NRFX_SUCCESS)
                    {
                        getDvStatus.setValue(String("OK!"));
                    }
                    else
                    {
                        getDvStatus.setValue(String("FAILED!"));
                    }
                }
                else if (String(getDvStatus.value()) == "getChargingStatus")
                {
                    getDvStatus.setValue(String("OK!"));
                    if(chargeState)
                    {
                        BatCharStat.setValue("CHARGING");
                    }
                    else
                    {
                        BatCharStat.setValue("DISCHARGING");
                    }
                }
                else
                {
                    if (Debug_Status != 0)
                    {
                        Serial.println("Invalid device.");
                    }
                    getDvStatus.setValue("INVALID DEVICE!");
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
                        DeviceToken.setValue("TOKEN OK!");
                        TokenModifyToken = true;
                        BLETimer = millis();
                    }
                    else
                    {
                        if (Debug_Status != 0)
                        {
                            Serial.println("Invalid reset token.");
                        }
                        DeviceToken.setValue("TOKEN FAILED!");
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
                        DeviceToken.setValue("MODIFIED OK!");
                        TokenModifyToken = false;
                    }
                    else
                    {
                        if (Debug_Status != 0)
                        {
                            Serial.println("Invalid token length.");
                        }
                        DeviceToken.setValue("TOKEN LENGTH FAILED!");
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
                    getPDMSmple.setValue("STREAMING OK!");
                }
                else if (String(getPDMSmple.value()) == "stopSamples")
                {
                    Serial.println("stop steaming.");
                    stop_record = true;  
                    getPDMSmple.setValue("STREAMING STOP!");
                }
                else
                {
                    getPDMSmple.setValue("Invalid command");
                    Serial.println("INVALID COMMAND!");
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
                    EmergencyNo.setValue("LIST FORMAT INVALID!");
                }
                else
                {
                    jsonData["EmergencyNo"] = EmergencyList;
                    bluetoothModified = true;
                    if (Debug_Status != 0)
                    {
                        Serial.println("Emergency list set!");
                    }
                    EmergencyNo.setValue("LIST OK!");
                }
            }
            if (AdminComman.written())
            {
                if (AdminComman.value() == "SysAdmin")
                {
                    AdminMode = !AdminMode;
                    AdminComman.setValue("OK!");
                }
                else if (AdminMode)
                {
                    String rtnVal = "OK!";
                    if (AdminComman.value() == "EnDebug0")
                    {
                        Debug_Status = 0;
                    }
                    else if (AdminComman.value() == "EnDebug1")
                    {
                        Debug_Status = 1;
                    }
                    else if (AdminComman.value() == "EnDebug2")
                    {
                        Debug_Status = 2;
                    }
                    else if (AdminComman.value() == "BatteryVoltage")
                    {
                        returnPercentage = false;
                    }
                    else if (AdminComman.value() == "BatteryPercentage")
                    {
                        returnPercentage = true;
                    }
                    else if (AdminComman.value() == "SlowCharging")
                    {
                        fastCharging = false;
                    }
                    else if (AdminComman.value() == "FastCharging")
                    {
                        fastCharging = true;
                    }
                    else
                    {
                        rtnVal = "FAILED!";
                    }
                    AdminComman.setValue(rtnVal);
                }
                else
                {
                    AdminComman.setValue("PERM FAILED!");
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
            broadcastBatteryBluetooth();
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
                BLESAuthNum.setValue("TIMEOUT!");
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
                    BLESAuthNum.setValue("BLUETOOTH OK!");
                    if (Debug_Status != 0)
                    {
                        Serial.println("Bluetooth Authentication Successful.");
                    }
                }
                else
                {
                    BLESAuthNum.setValue("BLUETOOTH FAILED!");
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
            DeviceToken.setValue("TIMEOUT!");
        }
    }
}

static void BLEInit()
{
    // Wait for BLE to startup
    if (!BLE.begin())
    {
        if (Debug_Status != 0)
        {
            Serial.println("Starting Bluetooth® Low Energy module failed!");
        }
        while (1);
    }
    if (Debug_Status != 0)
    {
        Serial.println("Started Bluetooth® Low Energy module!");
    }

    // Set BLE name
    BLE.setLocalName("Gebirah-Help");
    BLE.setDeviceName("P-Alarm-Device");
    // Set the advertised service
    BLE.setAdvertisedService(myService);

    // Add the service (with its characteristics) to the BLE server
    myService.addCharacteristic(BLESAuthNum);
    myService.addCharacteristic(EmergencyNo);
    myService.addCharacteristic(getPDMSmple);
    myService.addCharacteristic(PDMsMicRecs);
    myService.addCharacteristic(getDvStatus);
    myService.addCharacteristic(DeviceToken);
    myService.addCharacteristic(BtnCodeSend);
    myService.addCharacteristic(BatteryStat);
    myService.addCharacteristic(BatCharStat);
    myService.addCharacteristic(AdminComman);

    // Serial.println(BLESAuthNum.valueSize());
    // Serial.println(EmergencyNo.valueSize());
    // Serial.println(getPDMSmple.valueSize());
    // Serial.println(PDMsMicRecs.valueSize());
    // Serial.println(getDvStatus.valueSize());
    // Serial.println(DeviceToken.valueSize());
    // Serial.println(BtnCodeSend.valueSize());

    BLE.addService(myService);
    // Start advertising the device
    BLE.advertise();
    if (Debug_Status != 0)
    {
        Serial.println("BLE server is up and advertising!");
    }
}