#include <mic.h>

#include "Wire.h"
#include "LSM6DS3.h"
#include <ArduinoBLE.h>

// UUID for Alert Notification Service
BLEService myService("00001811-0000-1000-8000-00805F9B34FB");
// Set BLE to read and write (When phone is unlocked), and notify (when phone is on locked)

BLEStringCharacteristic FloatAccelX  ("00001811-0000-1000-8000-00805F9B34E1", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatAccelY  ("00001811-0000-1000-8000-00805F9B34E2", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatAccelZ  ("00001811-0000-1000-8000-00805F9B34E3", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatGyroX   ("00001811-0000-1000-8000-00805F9B34E4", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatGyroY   ("00001811-0000-1000-8000-00805F9B34E5", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatGyroZ   ("00001811-0000-1000-8000-00805F9B34E6", BLEWrite | BLENotify, 20);
BLEStringCharacteristic TempC        ("00001811-0000-1000-8000-00805F9B34E7", BLEWrite | BLENotify, 20);

// BLEStringCharacteristic UserName     ("00001811-0000-1000-8000-00805F9B34F8", BLERead | BLEWrite | BLENotify, 20);
// BLEStringCharacteristic UserAge      ("00001811-0000-1000-8000-00805F9B34F9", BLERead | BLEWrite | BLENotify, 20);

int eepromAddress = 0;

LSM6DS3 myIMU(I2C_MODE, 0x6A);    //I2C device address 0x6A

bool    centralDisconnected = false;
long    StartTimestamp = 0;

void setup() {
    pinMode(LED_GREEN, OUTPUT);
    // Begin serial communication and wait for serial communication
    Serial.begin(9600);
    // while (!Serial);
    // Print starting message
    Serial.println("Starting Device...");

    // Check for IMU 
    if (myIMU.begin() != 0) {
        Serial.println("Device error");
    } else {
        Serial.println("Device OK!");
    }
    
    // Wait for BLE to startup
    if (!BLE.begin()) {
        Serial.println("Starting Bluetooth® Low Energy module failed!");
        while (1);
    }

    // Set BLE name
    BLE.setLocalName("Gebirah-Help");
    BLE.setDeviceName("Personal-Alarm-Device");
    // Set the advertised service
    BLE.setAdvertisedService(myService);

    myService.addCharacteristic(FloatAccelX);
    myService.addCharacteristic(FloatAccelY);
    myService.addCharacteristic(FloatAccelZ);
    myService.addCharacteristic(FloatGyroX);
    myService.addCharacteristic(FloatGyroY);
    myService.addCharacteristic(FloatGyroZ);
    myService.addCharacteristic(TempC);    

    // myService.addCharacteristic(UserName);    

    BLE.addService(myService);

    // Start advertising the device
    BLE.advertise();
    Serial.println("BLE server is up and advertising!");
    digitalWrite(LED_GREEN, HIGH);   
}

void loop() {
    BLE.poll();
    // listen for Bluetooth® Low Energy peripherals to connect:
    BLEDevice central = BLE.central();

    // if a central is connected to peripheral:
    if (central) {
        Serial.print("Connected to central: ");
        // print the central's MAC address:
        Serial.println(central.address());
        centralDisconnected = true;
        StartTimestamp = millis();
    }

    // while the central is still connected to peripheral:
    while (central.connected()) {
        long timestamp = millis();
        if (timestamp - StartTimestamp >= 10)
        {
            // Your custom logic here
            FloatAccelX.setValue(String(timestamp) + "," + String(myIMU.readFloatAccelX()));
            FloatAccelY.setValue(String(timestamp) + "," + String(myIMU.readFloatAccelY()));
            FloatAccelZ.setValue(String(timestamp) + "," + String(myIMU.readFloatAccelZ()));
            FloatGyroX.setValue(String(timestamp) + "," + String(myIMU.readFloatGyroX()));
            FloatGyroY.setValue(String(timestamp) + "," + String(myIMU.readFloatGyroY()));
            FloatGyroZ.setValue(String(timestamp) + "," + String(myIMU.readFloatGyroZ()));
            TempC.setValue(String(timestamp) + "," + String(myIMU.readTempC()));
            StartTimestamp = millis();
        }
    }

    if (centralDisconnected)
    {
        // when the central disconnects, print it out:
        Serial.print(F("Disconnected from central: "));
        Serial.println(central.address());
        centralDisconnected = false;
    }
}