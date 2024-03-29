#include <SoftwareSerial.h>
// #include <StreamDevice.h>

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(2, 3); // SoftwareSerial(rxPin, txPin, inverse_logic)

void setup()
{
	//Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
	Serial.begin(9600);

	//Begin serial communication with Arduino and SIM800L
	mySerial.begin(115200);
    delay(1000);

	Serial.println("Starting Arduino...");
    mySerial.println("AT+BAUD4");
    // mySerial.println("AT");

    // Serial.println("Sending AT command to GSM module...");
    // sendATCommand("AT");
    // sendATCommand("AT+CREG=1");
    // sendATCommand("AT+COPS=1,2,\"e-ideas\"");
    // delay(1000);
    // // sendATCommand("AT+CCLK ?");
    // // delay(1000);
    // sendATCommand("AT+CMGF=1");
    // delay(1000);
    // sendATCommand("AT+CMGS=\"+6596188396\"");
    // delay(1000);
    // sendATCommand("SinKiat test, tele me if you received this.");
    // delay(1000);

    // sendATCommand("AT+CREG?");
    // sendATCommand("AT+CSPN?");
    // delay(3000);
    // sendATCommand("AT+CGATT=1");
    // delay(3000);
    // sendATCommand("AT+CGDCONT=1,\"IP\",\"CMNET\"");
    // delay(3000);
    // sendATCommand("AT+CGACT=1,1");
    // delay(3000);
    // sendATCommand("AT+HTTPGET=\"https://www.dfrobot.com\"");
    // // AT+RST=1 - Software reset
    // // AT+RST=0 - Power Off
    // Serial.println("Command: AT+RST=1");
	// mySerial.println("AT+RST=1");
	// updateSerial(true);

    // // Get network query
    // Serial.println("Command: AT+CREG=1");
    // mySerial.println("AT+CREG=1");
    // updateSerial(true);
    // Serial.println("Command: At+LOCATION=1");
    // mySerial.println("At+LOCATION=1");
    // updateSerial(true);

    // // Get Time From Base Station
    // Serial.println("Command: AT+CCLK?");
    // mySerial.println("AT+CCLK?");
    // updateSerial(true);

    // // Send Message
    // Serial.println("Command: AT+CMGF=1");
    // mySerial.println("AT+CMGF=1");
    // updateSerial(true);
    // Serial.println("Command: AT+CMGS=\"96188396\"");
    // mySerial.println("AT+CMGS=\"96188396\"");
    // updateSerial(true);
    // mySerial.println("SinKiat test, tele me if you received this.");
    // updateSerial(true);

    // // Get HTTP
    // Serial.println("Command: AT+CGATT=1");
    // mySerial.println("AT+CGATT=1");
    // updateSerial(true);
    // Serial.println("Command: AT+CGDCONT=1,\"IP\",\"CMNET\"");
    // mySerial.println("AT+CGDCONT=1,\"IP\",\"CMNET\"");
    // updateSerial(true);
    // Serial.println("Command: AT+CGACT=1,1");
    // mySerial.println("AT+CGACT=1,1");
    // updateSerial(true);
    // Serial.println("Command: AT+HTTPGET=\"https://www.dfrobot.com\"");
    // mySerial.println("AT+HTTPGET=\"https://www.dfrobot.com\"");
    // updateSerial(true);
}

void loop()
{
    sendATCommand();
}

// void sendATCommand(const char* command) {
void sendATCommand() 
{
    // mySerial.println(command);
    while (Serial.available())
    {
        mySerial.write(Serial.read());
        mySerial.flush();
        delay(1000);
    }
    while (mySerial.available()) 
    {
        String mSstring = mySerial.readString();
        Serial.println(mSstring);
        delay(1000);
    }
}