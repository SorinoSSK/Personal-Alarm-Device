#include <ArduinoBLE.h>

// UUID for Alert Notification Service
BLEService myService("00001811-0000-1000-8000-00805F9B34FB");
// Set BLE to read and write (When phone is unlocked), and notify (when phone is on locked)
BLEStringCharacteristic myCharacteristic("00001811-0000-1000-8000-00805F9B34FB", BLERead | BLEWrite | BLENotify, 20);

bool    centralDisconnected = false;
float   sensorValue         = 0.0;

void setup() {
    // Begin serial communication and wait for serial communication
    Serial.begin(9600);
    while (!Serial);
    // Print starting message
    Serial.println("Starting Device...");

    // Wait for BLE to startup
    if (!BLE.begin()) {
        Serial.println("Starting Bluetooth® Low Energy module failed!");
        while (1);
    }

    // Set BLE name
    BLE.setLocalName("Gebirah-Help");

    // Set the advertised service
    BLE.setAdvertisedService(myService);

    // Add the service (with its characteristics) to the BLE server
    myService.addCharacteristic(myCharacteristic);
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

        // Update sensor value
        sensorValue += 0.1;

        // Write the sensor value to the BLE characteristic
        // Serial.println(sensorValue);
        myCharacteristic.setValue(String(sensorValue));
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