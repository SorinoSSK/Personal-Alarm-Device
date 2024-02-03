// Include libraries
#include <mic.h>                            // Used for Microphone
#include "Wire.h"                           // Used for IMU
#include <stdio.h>                          // Used for type conversion
#include <string.h>                         // Used for using string
#include <stdlib.h>                         // Used for char string conversion
#include "LSM6DS3.h"                        // Used for IMU
#include "nrf_log.h"                        // Used for QSPI (ROM)
#include "nrf_delay.h"                      // Used for QSPI (ROM)
#include "nrfx_qspi.h"                      // Used for QSPI (ROM)
#include "sdk_config.h"                     // Used for QSPI (ROM)
#include <ArduinoBLE.h>                     // Used for Bluetooth Low Power
#include <ArduinoJson.h>                    // Used for using JSON
#include "nrf_log_ctrl.h"                   // Used for QSPI (ROM)
#include "avr/interrupt.h"                  // Used for QSPI (ROM)
#include "app_util_platform.h"              // Used for QSPI (ROM)
#include "nrf_log_default_backends.h"       // Used for QSPI (ROM)
#if defined(WIO_TERMINAL)                   
#include "processing/filters.h"             // Used for Microphone (PDM) Sampling
#endif

// ===== QSPI Settings ===== //
#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99
#define QSPI_DPM_ENTER      0x0003 // 3 x 256 x 62.5ns = 48ms
#define QSPI_DPM_EXIT       0x0003

// ===== Flash Memory Settings ===== //
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

// ===== IMU Settings ===== //
LSM6DS3 myIMU(I2C_MODE, 0x6A);      // I2C device address 0x6A

// ===== PDM Settings ===== //
#if defined(WIO_TERMINAL)
#define DEBUG 1                     // Enable pin pulse during ISR  
#define SAMPLES 16000*3
#elif defined(ARDUINO_ARCH_NRF52840)
#define DEBUG 1                     // Enable pin pulse during ISR  
#define SAMPLES 1600
#endif
mic_config_t mic_config{
    .channel_cnt = 1,
    .sampling_rate = 16000,
    #if defined(WIO_TERMINAL)
    .buf_size = 320,
    .debug_pin = 1                  // Toggles each DAC ISR (if DEBUG is set to 1)
    #elif defined(ARDUINO_ARCH_NRF52840)
    .buf_size = 1600,
    .debug_pin = LED_BUILTIN        // Toggles each DAC ISR (if DEBUG is set to 1)
    #endif
};
#if defined(WIO_TERMINAL)
DMA_ADC_Class Mic(&mic_config);
#elif defined(ARDUINO_ARCH_NRF52840)
NRF52840_ADC_Class Mic(&mic_config);
#endif
int8_t recording_buf[SAMPLES];
volatile uint8_t recording = 0;
volatile static bool record_ready = false;
volatile static bool stop_record = false;
#if defined(WIO_TERMINAL)
FilterBuHp filter;
#endif

// ===== Battery Settings ===== //
#define BAT_READ 14

// ===== Device Button Settings ===== //
uint8_t FirstBtnPin = 0;
bool FirstBtnStatus = false;
bool FirstBtnFirstPress = false;
bool FirstBtnRlseStatus = false;
long FirstBtnTimer = 0;
uint8_t SeconBtnPin = 1;
bool SeconBtnStatus = false;
bool SeconBtnFirstPress = false;
bool SeconBtnRlseStatus = false;
long SeconBtnTimer = 0;

// ===== Other Settings ===== //
DynamicJsonDocument jsonData(MemToUse);
uint32_t Error_Code;
uint32_t NoOfEmergencyContact = JSON_ARRAY_SIZE(10);
bool resetDevice = false;
bool bluetoothConnected = false;
bool bluetoothAuthenticated = false;
bool TokenModifyToken = false;
unsigned long BLETimer = millis();

// ===== Modifiable Settings ===== //
uint16_t Debug_Status       = 2;
uint16_t Bluetooth_Time_Out = 30*1000;
uint16_t Modify_Token_Time_Out = 30*1000;

// ===== Bluetooh Settings ===== //
// UUID for Alert Notification Service
BLEService myService("00001811-0000-1000-8000-00805F9B34E0");
// Set BLE to read and write (When phone is unlocked), and notify (when phone is on locked)
BLEStringCharacteristic BLESAuthNum("00001811-0000-1000-8000-00805F9B34F0", BLERead | BLEWrite | BLENotify, 100);
BLEStringCharacteristic EmergencyNo("00001811-0000-1000-8000-00805F9B34F1", BLERead | BLEWrite | BLENotify, 100);
BLEStringCharacteristic getPDMSmple("00001811-0000-1000-8000-00805F9B34F2", BLERead | BLEWrite | BLENotify, 25);
BLEStringCharacteristic getDvStatus("00001811-0000-1000-8000-00805F9B34F4", BLERead | BLEWrite | BLENotify, 25);
BLEStringCharacteristic DeviceToken("00001811-0000-1000-8000-00805F9B34F5", BLERead | BLEWrite | BLENotify, 100);
BLEStringCharacteristic BtnCodeSend("00001811-0000-1000-8000-00805F9B34F6", BLERead | BLENotify, 100);

BLECharacteristic       PDMsMicRecs("00001811-0000-1000-8000-00805F9B34F3", BLERead | BLENotify , mic_config.buf_size);

void setup()
{
    // Begin serial communication and wait for serial communicationMic
    Serial.begin(9600);
    // Configure the data receive callback
    if (Debug_Status != 0)
    {
        while (!Serial);
    }
    // Set battery read (Test after battery is soldered)
    // pinMode(BAT_READ, OUTPUT);
    // digitalWrite(BAT_READ, LOW);
    // Print starting message
    if (Debug_Status != 0)
    {
        Serial.println("Starting Device...");
    }
    // Setup Microphone (PDM)
    MICInit1();
    // Set Battery current
    pinMode(P0_13, OUTPUT);
    // Check for IMU Status
    IMUInit();
    // Initialise ROM
    QSPIInit();
    // Startup Microphone (PDM)
    MICInit2();
    // Setup and Startup BLE
    BLEInit();
    // Read from memory and initialise device
    // Else overwrite with default
    QSPIMemoryCheck();
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
}

static void readAllPins()
{
    debounce(FirstBtnPin, &FirstBtnStatus, &FirstBtnFirstPress, &FirstBtnRlseStatus, &FirstBtnTimer);
    debounce(SeconBtnPin, &SeconBtnStatus, &SeconBtnFirstPress, &SeconBtnRlseStatus, &SeconBtnTimer);
}