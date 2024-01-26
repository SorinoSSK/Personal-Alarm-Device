#include <mic.h>

#include "Wire.h"
#include "LSM6DS3.h"
#include <ArduinoBLE.h>

// UUID for Alert Notification Service
BLEService myService("00001811-0000-1000-8000-00805F9B34FB");
// Set BLE to read and write (When phone is unlocked), and notify (when phone is on locked)
BLEStringCharacteristic myCharacteristic("00001811-0000-1000-8000-00805F9B34FB", BLERead | BLEWrite | BLENotify, 20);

BLEStringCharacteristic FloatAccelX  ("00001811-0000-1000-8000-00805F9B34F1", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatAccelY  ("00001811-0000-1000-8000-00805F9B34F2", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatAccelZ  ("00001811-0000-1000-8000-00805F9B34F3", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatGyroX   ("00001811-0000-1000-8000-00805F9B34F4", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatGyroY   ("00001811-0000-1000-8000-00805F9B34F5", BLEWrite | BLENotify, 20);
BLEStringCharacteristic FloatGyroZ   ("00001811-0000-1000-8000-00805F9B34F6", BLEWrite | BLENotify, 20);
BLEStringCharacteristic TempC        ("00001811-0000-1000-8000-00805F9B34F7", BLEWrite | BLENotify, 20);

// BLEStringCharacteristic UserName     ("00001811-0000-1000-8000-00805F9B34F8", BLERead | BLEWrite | BLENotify, 20);
// BLEStringCharacteristic UserAge      ("00001811-0000-1000-8000-00805F9B34F9", BLERead | BLEWrite | BLENotify, 20);

int eepromAddress = 0;

LSM6DS3 myIMU(I2C_MODE, 0x6A);    //I2C device address 0x6A

bool    centralDisconnected = false;
float   sensorValue         = 0.0;

void setup() {
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

    // Add the service (with its characteristics) to the BLE server
    myService.addCharacteristic(myCharacteristic);

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
    }

    // while the central is still connected to peripheral:
    while (central.connected()) {
        // Your custom logic here
        // Check if the characteristic was written by the central device
        if (myCharacteristic.written()) {
            Serial.print("Received new value: ");
            Serial.println(String(myCharacteristic.value()));
        }
        // if(UserName.written()) {
        //     Serial.print("Received new value: ");
        //     Serial.println(String(UserName.value()));
        //     writeStringToEEPROM(0, String(UserName.value()));
        // }
        // if(UserAge.written()) {
        //     Serial.print("Received new value: ");
        //     Serial.println(String(UserAge.value()));
        //     writeStringToEEPROM(sizeof(int) + 20, String(UserAge.value()));
        // }
        // Update sensor value
        sensorValue += 0.1;

        // Write the sensor value to the BLE characteristic
        // Serial.println(sensorValue);
        myCharacteristic.setValue(String(sensorValue));
        
        FloatAccelX.setValue(String(myIMU.readFloatAccelX()));
        FloatAccelY.setValue(String(myIMU.readFloatAccelY()));
        FloatAccelZ.setValue(String(myIMU.readFloatAccelZ()));
        FloatGyroX.setValue(String(myIMU.readFloatGyroX()));
        FloatGyroY.setValue(String(myIMU.readFloatGyroY()));
        FloatGyroZ.setValue(String(myIMU.readFloatGyroZ()));
        TempC.setValue(String(myIMU.readTempC()));

        // String str1 = readStringFromEEPROM(0);
        // String str2 = readStringFromEEPROM(sizeof(int) + 20);

        // Serial.println("Name: ", str1);
        // Serial.println("Age: ", str2);

        // myCharacteristic.broadcast();
        delay(1000);
    }

    if (centralDisconnected)
    {
        // when the central disconnects, print it out:
        Serial.print(F("Disconnected from central: "));
        Serial.println(central.address());
        centralDisconnected = false;
    }
}

// void writeStringToEEPROM(int addr, const String& str) {
//     int length = str.length();
//     EEPROM.put(addr, length);  // Store the length of the string
//     addr += sizeof(int);

//     for (int i = 0; i < length; ++i) {
//         EEPROM.put(addr + i, str[i]);  // Store each character of the string
//     }
// }

// String readStringFromEEPROM(int addr) {
//     int length;
//     EEPROM.get(addr, length);  // Retrieve the length of the string
//     addr += sizeof(int);

//     String str;

//     for (int i = 0; i < length; ++i) {
//         char c;
//         EEPROM.get(addr + i, c);  // Retrieve each character of the string
//         str += c;
//     }

//     return str;
// }