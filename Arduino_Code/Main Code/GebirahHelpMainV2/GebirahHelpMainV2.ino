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

// ===== LED Settings ===== //
String statusForLED = "";
long LEDTimer       = 0;
bool LEDToReset     = false;

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
#define CONVERT_G_TO_MS2 9.80665f
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
    .debug_pin = 1
    // .debug_pin = LED_BUILTIN        // Toggles each DAC ISR (if DEBUG is set to 1)
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
#define VBAT_ENABLE         P0_14  // 
#define BAT_HIGH_CHARGE     22  // HIGH for 50mA, LOW for 100mA
#define BAT_CHARGE_STATE    23  // LOW for charging, HIGH not charging
#define ADC_Vref            3.3
#define Voltage_Div_Num     1510.0
#define Voltage_Div_Den     510.0
#define Voltage_Div_Offset  60.3
uint8_t chargeState     = 0;
long    batteryReadTime = 0;

// ===== Battery Kalman Filter ===== //
double estBatteryVal    = 0.0;   // x - Initial Guess
double estBatteryF      = 1.0;   // F - Only 1 Sensor
double estBatteryH      = 1.0;   // H - Only 1 Measurement Value
double estBatteryQ      = 0.1;          // Q - Near Calculated Value?
double estBatteryR      = 0.00001;      // R - Near Computation
double estBatteryKal    = 0.0;   // K
double estBatteryP      = 1.0;   // P

// ===== Device Button Settings ===== //
uint8_t FirstBtnPin     = 0;
bool FirstBtnStatus     = false;
bool FirstBtnFirstPress = false;
bool FirstBtnRlseStatus = false;
long FirstBtnTimer      = 0;
uint8_t SeconBtnPin     = 1;
bool SeconBtnStatus     = false;
bool SeconBtnFirstPress = false;
bool SeconBtnRlseStatus = false;
long SeconBtnTimer      = 0;

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
bool fastCharging               = true;
bool returnPercentage           = true;
uint16_t Debug_Status           = 0;
uint16_t Bluetooth_Time_Out     = 30*1000;
uint16_t Modify_Token_Time_Out  = 30*1000;

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
BLEStringCharacteristic BatteryStat("00001811-0000-1000-8000-00805F9B34F7", BLERead | BLENotify, 100);
BLEStringCharacteristic BatCharStat("00001811-0000-1000-8000-00805F9B34F8", BLERead | BLENotify, 100);

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
    // Initialise LED pin
    initLED();
    // Initialise Battery's Pin
    initBattery();
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
    if (Debug_Status != 0)
    {
        Serial.println("Device finish setting up...");
    }
    statusForLED = "completeSetup";
}

void loop()
{
    readAllPins();
    bluetoothFunction();
    // IMUFunction();
    if (resetDevice)
    {
        resetMemory();
        resetDevice = false;
    }
    runLED();
    // CheckBatteryChargingState();
    // readBattery();
}

static void readAllPins()
{
    debounce(FirstBtnPin, &FirstBtnStatus, &FirstBtnFirstPress, &FirstBtnRlseStatus, &FirstBtnTimer);
    debounce(SeconBtnPin, &SeconBtnStatus, &SeconBtnFirstPress, &SeconBtnRlseStatus, &SeconBtnTimer);
}