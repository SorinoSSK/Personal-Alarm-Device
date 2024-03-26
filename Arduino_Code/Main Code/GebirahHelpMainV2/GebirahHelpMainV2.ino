#include <mic.h>

// Include libraries
// #include "Arduino.h"
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
#include "NRF52_MBED_TimerInterrupt.h"
#if defined(WIO_TERMINAL)                   
#include "processing/filters.h"             // Used for Microphone (PDM) Sampling
#endif

// ===== Buzzer Settings =====//
unsigned long beepTimer = 0;
uint8_t BuzzerPin       = 9;
bool BuzzerState        = false;

// ===== LED Settings ===== //
String statusForLED         = "";
uint8_t LED_BLE             = 3;
uint8_t LED_FIRST_BTN       = 5;
uint8_t LED_SECON_BTN       = 4;
unsigned long LEDTimer      = 0;
unsigned long SideLEDTimer  = 0;
bool SideLEDState           = false;
bool LEDToReset             = false;

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
const int IMUBufferSize1            = 10;
float IMUVal[IMUBufferSize1][6]     = {{0},{0}};
const int IMUBufferSize2            = 2;
float IMUDiffVal[IMUBufferSize2]    = {0};
const int IMUBufferSize3            = 10;
float IMUBuffVal[IMUBufferSize3]    = {0};
uint8_t IMUFallDetectedBlinkRate    = 500;
bool IMUFallDetectedLEDLight        = false;
bool IMUFallDetected                = false;
bool IMUFallDetectedSent            = false;
unsigned long IMUBlinkTimer         = 0;

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
    .debug_pin = 10                  // Toggles each DAC ISR (if DEBUG is set to 1)
    #elif defined(ARDUINO_ARCH_NRF52840)
    .buf_size = 1600,
    .debug_pin = 10
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
const int sizeOfFilter          = 10;
uint8_t chargeState             = 0;
unsigned long batteryReadTime   = 0;
int     filterCnt               = 0;
float   pastBattVoltage[sizeOfFilter] = {0};
float   BatteryVoltage          = 0;
// bool    BatterySoftLimit    = true;
bool    BatteryReadingRdy       = false;

// // ===== Battery Kalman Filter ===== //
// double estBatteryVal    = 0.0;   // x - Initial Guess
// double estBatteryF      = 1.0;   // F - Only 1 Sensor
// double estBatteryH      = 1.0;   // H - Only 1 Measurement Value
// double estBatteryQ      = 0.1;          // Q - Near Calculated Value?
// double estBatteryR      = 0.00001;      // R - Near Computation
// double estBatteryKal    = 0.0;   // K
// double estBatteryP      = 1.0;   // P

// ===== Device Button Settings ===== //
uint8_t FirstBtnPin     = 0;
bool FirstBtnStatus     = false;
bool FirstBtnStatusSent = false;
bool FirstBtnFirstPress = false;
bool FirstBtnRlsePress  = false;
unsigned long FirstBtnTimer      = 0;
uint8_t SeconBtnPin     = 1;
bool SeconBtnStatus     = false;
bool SeconBtnStatusSent = false;
bool SeconBtnFirstPress = false;
bool SeconBtnRlsePress  = false;
unsigned long SeconBtnTimer      = 0;
uint8_t ThirdBtnPin     = 2;
bool ThirdBtnStatus     = false;
bool ThirdBtnFirstPress = false;
bool ThirdBtnRlseStatus = false;
unsigned long ThirdBtnTimer      = 0;

// ===== Other Settings ===== //
DynamicJsonDocument jsonData(MemToUse);
uint32_t Error_Code;
uint32_t NoOfEmergencyContact   = JSON_ARRAY_SIZE(10);
bool resetDevice                = false;
bool bluetoothConnected         = false;
bool bluetoothAuthenticated     = false;
bool bluetoothAuthenticated2    = false;
bool TokenModifyToken           = false;
bool resendSOS                  = false;
bool FirstBtnTriggered          = false;
bool SeconBtnTriggered          = false;
unsigned long BLETimer          = millis();
unsigned long sysAdminTimer     = millis();
unsigned long resendSOSTimer    = millis();

// ===== Modifiable Settings ===== //
bool AdminMode                  = false;
// bool fastCharging               = true;
// bool returnPercentage           = true;
float lowerAccelTresh           = 3.5;
uint16_t Fall_Detected          = 0;
uint16_t Debug_Status           = 2;
uint16_t sysAdmin_Time_Out      = 60*60*1000;
uint16_t Bluetooth_Time_Out     = 30*1000;
uint16_t Modify_Token_Time_Out  = 30*1000;
uint8_t  Resend_SOS_Time_Out    = 30*1000;

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
BLEStringCharacteristic AdminComman("00001811-0000-1000-8000-00805F9B34F9", BLERead | BLEWrite | BLENotify, 100);
BLEStringCharacteristic BleRSSIValu("00001811-0000-1000-8000-00805F9B34FA", BLERead | BLENotify, 100);

BLECharacteristic       PDMsMicRecs("00001811-0000-1000-8000-00805F9B34F3", BLERead | BLENotify , mic_config.buf_size);
const int sizeOfRSSIFilter              = 10;
int filterCntRSSI                       = 0;
float pastRSSIValue[sizeOfRSSIFilter]   = {0};

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
    // Initialise ROM
    QSPIInit();
    // Read from memory and initialise device
    // Else overwrite with default
    QSPIMemoryCheck();
    // Initialise LED pin
    initLED();
    // Initialise Buzzer's Pin
    initBuzzer();
    // Initialise Battery's Pin
    initBattery();
    // Setup Microphone (PDM)
    MICInit1();
    // Set Battery current
    pinMode(P0_13, OUTPUT);
    // Check for IMU Status
    IMUInit();

    // Startup Microphone (PDM)
    MICInit2();
    // Setup and Startup BLE
    BLEInit();
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
    IMUFunction();
    if (resetDevice)
    {
        resetMemory();
        resetDevice = false;
    }
    runLED();
    runBattery();
    soundBuzzer();
    resetDeviceFunc();
}

static void readAllPins()
{
    debounceLatch(FirstBtnPin, &FirstBtnStatus, &FirstBtnFirstPress, &FirstBtnRlsePress, &FirstBtnTimer, &FirstBtnStatusSent);
    debounceLatch(SeconBtnPin, &SeconBtnStatus, &SeconBtnFirstPress, &SeconBtnRlsePress, &SeconBtnTimer, &SeconBtnStatusSent);
    debounce(ThirdBtnPin, &ThirdBtnStatus, &ThirdBtnFirstPress, &ThirdBtnRlseStatus, &ThirdBtnTimer);
}

static void resetDeviceFunc()
{
    if (ThirdBtnStatus)
    {
        resetMemory();
        ThirdBtnStatus = false;
    }
}