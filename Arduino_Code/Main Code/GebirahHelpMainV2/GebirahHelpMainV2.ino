#include "Wire.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "LSM6DS3.h"
#include "nrf_log.h"
#include "nrf_delay.h"
#include "nrfx_qspi.h"
#include "sdk_config.h"
#include <ArduinoBLE.h>
#include <ArduinoJson.h>
#include "nrf_log_ctrl.h"
#include "avr/interrupt.h"
#include "app_util_platform.h"
#include "nrf_log_default_backends.h"

// QSPI Settings
#define QSPI_STD_CMD_WRSR 0x01
#define QSPI_STD_CMD_RSTEN 0x66
#define QSPI_STD_CMD_RST 0x99
#define QSPI_DPM_ENTER 0x0003  // 3 x 256 x 62.5ns = 48ms
#define QSPI_DPM_EXIT 0x0003

// Bluetooh Settings
// UUID for Alert Notification Service
BLEService myService("00001811-0000-1000-8000-00805F9B34FB");
// Set BLE to read and write (When phone is unlocked), and notify (when phone is on locked)
BLEStringCharacteristic EmergencyNo("00001811-0000-1000-8000-00805F9B34F0", BLERead | BLEWrite | BLENotify, 100);

// Flash Memory Settings
static nrfx_qspi_config_t QSPIConfig;
static nrf_qspi_cinstr_conf_t QSPICinstr_cfg;
// Setup for the SEEED XIAO BLE - nRF52840
static uint32_t *QSPI_Status_Ptr = (uint32_t *)0x40029604;
// Alter this to create larger read writes, 64Kb is the size of the Erase
static const uint32_t MemToUse = 64 * 1024;
static bool Debug_On = true;
static bool QSPIWait = false;
// Usable size = 64Kb, where 1 character = 2 byte. Therefore total usable size = 32768 Characters
// 16bit used as that is what this memory is going to be used for
static uint16_t pBuf[MemToUse / 2] = { 0 };
static uint32_t *BufMem = (uint32_t *)&pBuf;

// IMU Settings
LSM6DS3 myIMU(I2C_MODE, 0x6A);  //I2C device address 0x6A

// Other Settings
DynamicJsonDocument jsonData(MemToUse);
// StaticJsonDocument<MemToUse> jsonData;
uint32_t Error_Code;
bool centralDisconnected = false;

void setup() {
  // Begin serial communication and wait for serial communication
  Serial.begin(9600);
  // Print starting message
  Serial.println("Starting Device...");

  // Check for IMU
  if (myIMU.begin() != 0) {
    Serial.println("IMU error");
  } else {
    Serial.println("IMU OK!");
  }

  if (QSPI_Initialise() != NRFX_SUCCESS) {
    Serial.println("(Setup) QSPI Memory failed to start!");
  } else {
    Serial.println("(Setup) QSPI initialised and ready");
    QSPI_Status("Setup (After initialise)");
  }

  // Wait for BLE to startup
  if (!BLE.begin()) {
    Serial.println("Starting Bluetooth® Low Energy module failed!");
    while (1)
      ;
  }
  Serial.println("Started Bluetooth® Low Energy module!");

  // Set BLE name
  BLE.setLocalName("Gebirah-Help");
  BLE.setDeviceName("Personal-Alarm-Device");
  // Set the advertised service
  BLE.setAdvertisedService(myService);
  BLE.addService(myService);

  // Add the service (with its characteristics) to the BLE server
  myService.addCharacteristic(EmergencyNo);

  // Start advertising the device
  BLE.advertise();
  Serial.println("BLE server is up and advertising!");

  // Wait for Memory to be ready for reading
  QSPI_WaitForReady();
  Error_Code = nrfx_qspi_read(pBuf, MemToUse, 0x0);
  // Wait for Memory to be ready after reading
  QSPI_WaitForReady();
  // If Error Code is 0 do work, else throw an error and proceed
  if (Error_Code == 0) {
    Serial.println("Initialising Data...");
    // DeserializationError error = deserializeJson(jsonData, (char *)&pBuf);
    DeserializationError error = deserializeJson(jsonData, uint16ToString(pBuf));
    // Check if data is in JSON format. If not then attempt to override with one.
    if (error) {
      // Print Error
      Serial.print(F("JSON deserialisation failed: "));
      Serial.println(error.f_str());
      // If Debugging mode, turn on full print.
      if (Debug_On) {
        Serial.println(F("Reading stored data..."));
        QSPI_PrintDataChar(&pBuf[0]);
      }
      // Initialise JSON and write it into JSON string
      Serial.println(F("Replacing JSON and writing into memory..."));
      jsonData["DeviceToken"] = "6814a608-5385-4ce1-917d-c51840bb75a0";
      jsonData["DResetToken"] = "52702af6-87d2-4290-a01b-036c88ec2811";
      jsonData["EmergencyNo"] = [];
      storeJSONToMemory();
    } else {
      // If successful, print ok.
      Serial.println("Memory, ok");
      // If debug, print JSON string
      if (Debug_On) {
        String jsonString;
        serializeJson(jsonData, jsonString);
        Serial.println(jsonString);
      }
    }
  } else {
    Serial.println("Error reading from Flash Memory! Skipping memory...");
  }
}

void loop() {
  bool bluetoothModified = false;
  BLE.poll();
  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();
  // if a central is connected to peripheral:
  if (central) {
    Serial.print(central.address());
    // print the central's MAC address:
    Serial.println(" connected to bluetooth.");
    centralDisconnected = true;
  }

  // while the central is still connected to peripheral:
  while (central.connected()) {
    // Your custom logic here
    // Check if the characteristic was written by the central device
    if (EmergencyNo.written()) {
      Serial.print("Received Emergency No.: ");
      Serial.println(String(EmergencyNo.value()));
      jsonData["EmergencyNo"] = String(EmergencyNo.value());
      bluetoothModified = true;
    }
    if (bluetoothModified) {
      storeJSONToMemory();
      bluetoothModified = false;
    }

    // Write the sensor value to the BLE characteristic
    // Serial.println(sensorValue);

    // FloatAccelX.setValue(String(myIMU.readFloatAccelX()));
    // FloatAccelY.setValue(String(myIMU.readFloatAccelY()));
    // FloatAccelZ.setValue(String(myIMU.readFloatAccelZ()));
    // FloatGyroX.setValue(String(myIMU.readFloatGyroX()));
    // FloatGyroY.setValue(String(myIMU.readFloatGyroY()));
    // FloatGyroZ.setValue(String(myIMU.readFloatGyroZ()));
    // TempC.setValue(String(myIMU.readTempC()));

    // String str1 = readStringFromEEPROM(0);
    // String str2 = readStringFromEEPROM(sizeof(int) + 20);

    // Serial.println("Name: ", str1);
    // Serial.println("Age: ", str2);

    // myCharacteristic.broadcast();
    delay(1000);
  }

  if (centralDisconnected) {
    // when the central disconnects, print it out:
    Serial.print(central.address());
    Serial.println(" disconnected from bluetooth.");
    centralDisconnected = false;
  }
}

static void storeJSONToMemory() {
  String jsonString;
  serializeJson(jsonData, jsonString);
  // Print String only it debugging
  if (Debug_On) {
    Serial.println(jsonString);
  }
  // Wait Memory to be ready for erasing and then writing.
  QSPI_WaitForReady();
  QSPI_Erase(0);
  QSPI_WaitForReady();
  // Convert JSON string back to unsigned int 16 for writing
  stringToUint16(jsonString, pBuf);
  if (Debug_On) {
    QSPI_PrintDataChar(&pBuf[0]);
  }
  // Write JSON into memory
  Error_Code = nrfx_qspi_write(pBuf, MemToUse, 0x0);
  QSPI_WaitForReady();
  // If no error, indicate successful write. Else indicate fail write and proceed.
  if (Error_Code == 0) {
    Serial.println("Data written successfully.");
  } else {
    Serial.println("Data write failed.");
  }
}