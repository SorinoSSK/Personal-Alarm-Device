#include <mic.h>
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
#include "processing/filters.h"
#include "nrf_log_default_backends.h"

// QSPI Settings
#define QSPI_STD_CMD_WRSR 0x01
#define QSPI_STD_CMD_RSTEN 0x66
#define QSPI_STD_CMD_RST 0x99
#define QSPI_DPM_ENTER 0x0003 // 3 x 256 x 62.5ns = 48ms
#define QSPI_DPM_EXIT 0x0003

// Flash Memory Settings
static nrfx_qspi_config_t           QSPIConfig;
static nrf_qspi_cinstr_conf_t       QSPICinstr_cfg;
// Setup for the SEEED XIAO BLE - nRF52840
static uint32_t *QSPI_Status_Ptr    = (uint32_t *)0x40029604;
// Alter this to create larger read writes, 64Kb is the size of the Erase
static const uint32_t MemToUse      = 64 * 1024;
static bool QSPIWait                = false;
// Usable size = 64Kb, where 1 character = 2 byte. Therefore total usable size = 32768 Characters
// 16bit used as that is what this memory is going to be used for
static uint16_t pBuf[MemToUse / 2]  = {0};
static uint32_t *BufMem             = (uint32_t *)&pBuf;

// IMU Settings
LSM6DS3 myIMU(I2C_MODE, 0x6A); // I2C device address 0x6A

// PDM Settings
#define DEBUG 1                 // Enable pin pulse during ISR  
#define SAMPLES 16000*15        // 15 = 15 seconds
mic_config_t mic_config{
  .channel_cnt = 1,
  .sampling_rate = 16000,
  .buf_size = 128,
  .debug_pin = LED_BUILTIN                // Toggles each DAC ISR (if DEBUG is set to 1)
};
NRF52840_ADC_Class Mic(&mic_config);
short sampleBuffer[128];
// Number of audio samples read
volatile int samplesRead = 0;
uint32_t sample_cnt = 0;
FilterBuHp filter;

// Other Settings
DynamicJsonDocument jsonData(MemToUse);
// StaticJsonDocument<MemToUse> jsonData;
uint32_t Error_Code;
uint32_t NoOfEmergencyContact = JSON_ARRAY_SIZE(10);
bool MicRecord = false;
bool MicRecordRdy = false;
bool SendMicRecord = false;
bool resetDevice = false;
bool bluetoothConnected = false;
bool bluetoothAuthenticated = false;
unsigned long BLETimer = millis();

// Settings
uint16_t Debug_Status       = 0;
uint16_t Bluetooth_Time_Out = 30*1000;

// Bluetooh Settings
// UUID for Alert Notification Service
BLEService myService("00001811-0000-1000-8000-00805F9B34E0");
// Set BLE to read and write (When phone is unlocked), and notify (when phone is on locked)
BLEStringCharacteristic BLESAuthNum("00001811-0000-1000-8000-00805F9B34F0", BLERead | BLEWrite | BLENotify, 100);
BLEStringCharacteristic EmergencyNo("00001811-0000-1000-8000-00805F9B34F1", BLERead | BLEWrite | BLENotify, 100);
BLEStringCharacteristic getPDMSmple("00001811-0000-1000-8000-00805F9B34F2", BLERead | BLEWrite | BLENotify, 25);

// BLECharacteristic PDMsMicRecs("00001811-0000-1000-8000-00805F9B34F3", BLERead, sizeof(short)*SAMPLES);
BLECharacteristic       PDMsMicRecs("00001811-0000-1000-8000-00805F9B34F3", BLERead | BLENotify , mic_config.buf_size);

void setup()
{
    // Begin serial communication and wait for serial communicationMic
    Serial.begin(9600);
    // Configure the data receive callback
    Mic.set_callback(audio_rec_callback);
    if (Debug_Status != 0)
    {
        while (!Serial);
    }

    // Print starting message
    if (Debug_Status != 0)
    {
        Serial.println("Starting Device...");
    }

    // Check for IMU
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

    if (QSPI_Initialise() != NRFX_SUCCESS)
    {
        if (Debug_Status != 0)
        {
            Serial.println("QSPI Memory failed to start!");
        }
      }
    else
    {
        if (Debug_Status != 0)
        {
            Serial.println("QSPI initialised and ready!");
        }
    }

    if (!Mic.begin()) 
    {
        if (Debug_Status != 0)
        {
            Serial.println("Starting PDM MIC failed!");
        }
        while (1);
    }
    if (Debug_Status != 0)
    {
        Serial.println("Started PDM MIC!");
    }

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
    myService.addCharacteristic(PDMsMicRecs);
    myService.addCharacteristic(getPDMSmple);

    BLE.addService(myService);
    // Start advertising the device
    BLE.advertise();
    if (Debug_Status != 0)
    {
        Serial.println("BLE server is up and advertising!");
    }

    // Wait for Memory to be ready for reading
    QSPI_WaitForReady();
    Error_Code = nrfx_qspi_read(pBuf, MemToUse, 0x0);
    // Wait for Memory to be ready after reading
    QSPI_WaitForReady();
    // If Error Code is 0 do work, else throw an error and proceed
    if (Error_Code == 0)
    {
        if (Debug_Status != 0)
        {
            Serial.println("Initialising Data...");
        }
        // DeserializationError error = deserializeJson(jsonData, (char *)&pBuf);
        DeserializationError error = deserializeJson(jsonData, uint16ToString(pBuf));
        // Check if data is in JSON format. If not then attempt to override with one.
        if (error)
        {
            // Print Error
            if (Debug_Status != 0)
            {
                Serial.print(F("JSON deserialisation failed: "));
                Serial.println(error.f_str());
            }
            // If Debugging mode, turn on full print.
            if (Debug_Status == 2)
            {
                Serial.println(F("Reading stored data..."));
                QSPI_PrintDataChar(&pBuf[0]);
            }
            // Initialise JSON and write it into JSON string
            if (Debug_Status != 0)
            {
                Serial.println(F("Replacing JSON and writing into memory..."));
            }
            resetMemory();
        }
        else
        {
            // If successful, print ok.
            if (Debug_Status != 0)
            {
                Serial.println("Data in memory is normal!");
            }
            // If debug, print JSON string
            if (Debug_Status == 2)
            {
                String jsonString;
                serializeJson(jsonData, jsonString);
                Serial.println(jsonString);
            }
        }
    }
    else
    {
        if (Debug_Status != 0)
        {
            Serial.println("Error reading from Flash Memory! Skipping memory...");
        }
    }
}

void loop()
{
    readAllPins();
    bluetoothFunction();
    if (resetDevice)
    {
        resetMemory();
        resetDevice = false;
    }
    // microphoneFunction();
}

static void readAllPins()
{

}