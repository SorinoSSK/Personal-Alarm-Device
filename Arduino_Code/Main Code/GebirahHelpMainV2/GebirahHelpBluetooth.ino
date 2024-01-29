void bluetoothFunction() {
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
            if (getPDMSmple.written())
            {
                if (Debug_Status != 0)
                {
                    Serial.print("Received request for PDM record: ");
                    Serial.println(String(getPDMSmple.value()));
                }
                if (String(getPDMSmple.value()) == "getSamples")
                {
                    SendMicRecord = true;
                }
                else if (String(getPDMSmple.value()) == "stopSamples")
                {
                    SendMicRecord = false;
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
            
    }
    else
    {
        // if there is a bluetooth connection and is disconnected, reset status
        if (bluetoothConnected)
        {
            bluetoothConnected = false;
            bluetoothAuthenticated = false;
            if (Debug_Status != 0)
            {
                Serial.print("Bluetooth disconnected from device ");
                Serial.println(central.address());
            }
        }
    }
}
