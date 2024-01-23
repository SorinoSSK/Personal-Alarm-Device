/*
 * Project myProject
 * Author: Your Name
 * Date:
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
// #include "CellularHelper.h"
#include "dct.h"

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

// setup() runs once, when the device is first turned on
void setup()
{
    Serial.begin(9600);
    Serial.printlnf("Starting Particle Boron.");
    Serial.printlnf("Device OS version: %s", (const char*)System.version());
    Serial.printlnf("Initialising Cellular to External SIM...");
    Cellular.setActiveSim(EXTERNAL_SIM);
    Cellular.setCredentials("e-ideas");

    // This clears the setup done flag on brand new devices so it won't stay in listening mode
    const uint8_t val = 0x01;
    dct_write_app_data(&val, DCT_SETUP_DONE_OFFSET, 1);

    Serial.printlnf("Cellular Configured Successfully...");
    Serial.printlnf("Turning On Cellular.");
    Cellular.on();
    waitFor(Cellular.isOn, 30000);
    Cellular.connect();
    // Put initialization like pinMode and begin functions here
    Serial.printlnf("Connect to Particle Cloud...");
    Particle.connect();
    Log.info("Starting Particle Boron.");
    Log.info("Device OS version: %s", (const char*)System.version());
    delay(1000);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
    // if (Cellular.ready())
    // {
    //     // Serial.printlnf("Attempting to connect to cellular.");
    //     Log.info("Attempting to connect to cellular.");
    // }
    // else
    // {
    //     // Serial.printlnf("Successfully connected.");
    //     Log.info("Successfully connected.");
    // }
    Log.info("Cellular.connecting(): %d", Cellular.connecting());
    Log.info("Cellular.ready(): %d", Cellular.ready());
    // The core of your code will likely live here.

    // Example: Publish event to cloud every 10 seconds. Uncomment the next 3 lines to try it!
    // Log.info("Sending Hello World to the cloud!");
    // Particle.publish("Hello world!");
    // delay( 10 * 1000 ); // milliseconds and blocking - see docs for more info!
}
