#include <SPI.h>        
#include <Ethernet.h>

#define echoPin 7 // Echo Pin
#define triggerPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED

byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x28, 0xF7 };
byte ipAddress[] = {192, 168, 1, 103};
unsigned int serverPort = 5001;      // local port to listen on

byte googleServer[] = { 64, 233, 187, 99 }; // Google

int minimumRange = 0; // Minimum distance of the sensor
int doorWidth = 90; // Maximum width of the door
long duration, distance; // Duration used to calculate distance
bool shouldCount = 1;

EthernetClient client;

void setup() {
    Serial.begin (9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }

    //Setup sonar and LED indicator
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(LEDPin, OUTPUT); // Use LED indicator (will only be used to debug)

  // start the Ethernet connection:
    if (Ethernet.begin(mac) == 0) {
        Serial.println("Failed to configure Ethernet using DHCP");
        // try to congifure using IP address instead of DHCP:
        Ethernet.begin(mac, ipAddress);
    }
    // give the Ethernet shield a second to initialize:
    delay(1000);
    Serial.println("Connecting...");

    // if you get a connection, report back via serial:
    int connectionStatus = client.connect(ipAddress, serverPort);
  
    if (connectionStatus == 1) {
        Serial.println("Connected!");
    } else {
        // if you didn't get a connection to the server:
        Serial.println("Connection failed with status: ");
        Serial.print(connectionStatus);
    }
}

void loop() {

//    TESTS
//    if (Serial.available() > 0) { //checks for errors
//        int incomingByte = Serial.read();
//        Serial.println(incomingByte);
//        if (incomingByte > 0) {
//            client.println("RA:00000000,isDoor:1,isPresent:1");
//        }
//        Serial.flush();
//    }

    // if the server's disconnected, stop the client:
    if (!client.connected()) {
        Serial.println();
        Serial.println("Disconnecting.");
        client.stop();

        // do nothing forevermore:
        while (true);
    }
  
    /* The following triggerPin/echoPin cycle is used to determine the
     distance of the nearest object by bouncing soundwaves off of it. */
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    
    digitalWrite(triggerPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    
    //Calculate the distance (in cm) based on the speed of sound.
    distance = duration/58.2;

    if (shouldCount) {
        if (distance >= doorWidth || distance <= minimumRange) { //if the range is out of the bounds
            /*
             * This shouldn't do anything. Just log some error.
             */
            Serial.println("-1");
        } else if (distance < 60) { //if the range is within the defined bounds
            shouldCount = false; //it shouldn't count until the person has passed through the door
            
            Serial.println(distance); //prints the distance
            digitalWrite(LEDPin, LOW);
            client.println("RA:00000000,isDoor:1,isPresent:1");
        } else {
            shouldCount = true;    
            digitalWrite(LEDPin, HIGH);
        }
    } else {
        if (distance > 60) {
            shouldCount = true;
            digitalWrite(LEDPin, HIGH);
        }
    }
    
    //Delay 50ms before next reading.
    delay(50);
}
