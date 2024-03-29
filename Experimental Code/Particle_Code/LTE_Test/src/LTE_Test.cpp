/*
 * Project myProject
 * Author: Your Name
 * Date:
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "dct.h"
#include "HttpClient.h"
// #include "application.h"

unsigned int nextTime = 0; // Next time to contact the server
HttpClient http;
http_request_t request;
http_response_t response;
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    {"Accept", "*/*"},
    {NULL, NULL} // NOTE: Always terminate headers will NULL
};

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(LOG_LEVEL_INFO);

CellularBand band_set;
unsigned int CellularStatus = 0;

// setup() runs once, when the device is first turned on
void setup()
{
    Serial.begin(9600);
    Log.info("Starting Particle Boron.");
    Log.info("Device OS version: %s", (const char *)System.version());
    Log.info("Initialising Cellular to External SIM...");
    Cellular.setActiveSim(EXTERNAL_SIM);
    STARTUP(cellular_credentials_set("e-ideas", "", "", NULL));
    // Cellular.setCredentials("e-ideas");

    // This clears the setup done flag on brand new devices so it won't stay in listening mode
    const uint8_t val = 0x01;
    dct_write_app_data(&val, DCT_SETUP_DONE_OFFSET, 1);

    Log.info("Cellular Configured Successfully...");
    SimType simType = Cellular.getActiveSim();
    Log.info("simType=%d", simType);
    Log.info("Linking Particle...");

    // Cellular Docs
    // https://docs.particle.io/reference/device-os/firmware/#cellular
    band_set.band[0] = BAND_900;
    band_set.band[1] = BAND_1800;
    // band_set.band[2] = BAND_2600;
    band_set.count = 2;
    Cellular.setBandSelect(band_set);
    Cellular.on();

    // Serial.printlnf("Turning On Cellular.");
    // Cellular.on();
    // waitFor(Cellular.isOn, 30000);
    // Cellular.connect();
    // // Put initialization like pinMode and begin functions here
    // Serial.printlnf("Connect to Particle Cloud...");
    // Particle.connect();
    // Log.info("Starting Particle Boron.");
    // Log.info("Device OS version: %s", (const char*)System.version());
    // delay(1000);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
    if (CellularStatus == 0)
    {
        if (Cellular.isOn())
        {
            Log.info("Cellular is ready.");
            CellularStatus = 1;
        }
    }
    else if (CellularStatus == 1)
    {
        Log.info("Connecting to cellular network.");
        Cellular.connect();
        CellularStatus = 2;
    }
    else if (CellularStatus == 2)
    {
        // Log.info("Is cellular connecting: %d", Cellular.connecting());
        if (Cellular.ready())
        {
            Log.info("Cellular network is ready.");
            Log.info("Requesting API!");
            request.hostname = "https://api.seow-sinkiat.com/status/testapi";
            request.port = 80;
            http.get(request, response, headers);
            Log.info("status: %d", response.status);
            if (response.status == 200)
            {
                Log.info(response.body);
            }
            delay(5000);
        }
    }
    else
    {
        Log.info("Cellular is not connected!");
    }
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
    // Log.info("Cellular.connecting(): %d", Cellular.connecting());
    // Log.info("Cellular.ready(): %d", Cellular.ready());
    // The core of your code will likely live here.

    // Example: Publish event to cloud every 10 seconds. Uncomment the next 3 lines to try it!
    // Log.info("Sending Hello World to the cloud!");
    // Particle.publish("Hello world!");
    // delay( 10 * 1000 ); // milliseconds and blocking - see docs for more info!
}
