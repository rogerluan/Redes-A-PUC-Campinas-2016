#include <SPI.h>        
#include <Ethernet.h>

#define echoPin 7 // Echo Pin
#define triggerPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x28, 0xF7 };
IPAddress server(172, 16, 3, 77);

unsigned int serverPort = 5000;      // local port to listen on

//EthernetUDP UDP; // An EthernetUDP instance to let us send and receive packets over UDP
int minimumRange = 0; // Minimum distance of the sensor
int maximumRange = 90; // Maximum width of the door
long duration, distance; // Duration used to calculate distance

EthernetClient client;

void setup() {
    //setup sonar sensor and LED
    Serial.begin (9600);
    while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
    }

//    pinMode(triggerPin, OUTPUT);
//    pinMode(echoPin, INPUT);
//    pinMode(LEDPin, OUTPUT); // Use LED indicator (will only be used to debug)

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, server);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("Connecting...");

  // if you get a connection, report back via serial:
  if (client.connect(server, serverPort)) {
    Serial.println("Connected");
  } else {
    // if you didn't get a connection to the server:
    Serial.println("Connection failed");
  }
}

void loop() {

    if (Serial.available() > 0) { //checks for errors
        int incomingByte = Serial.read();
        Serial.flush();
        if (incomingByte == 1) {
            client.println("RA:00000000,isDoor:1,isPresent:1");
        }
    }

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
//    digitalWrite(triggerPin, LOW);
//    delayMicroseconds(2);
//    
//    digitalWrite(triggerPin, HIGH);
//    delayMicroseconds(10);
//    
//    digitalWrite(triggerPin, LOW);
//    duration = pulseIn(echoPin, HIGH);
//    
//    //Calculate the distance (in cm) based on the speed of sound.
//    distance = duration/58.2;
//    
//    if (distance >= maximumRange || distance <= minimumRange) { //if the range is out of the bounds
//        /*
//         * This shouldn't do anything. Just log some error (or light a LED).
//         */
//        Serial.println("-1");
//        digitalWrite(LEDPin, HIGH);
//    } else { //if the range is within the defined bounds
//        /* Send the distance to the computer using Serial protocol, and
//         turn LED OFF to indicate successful reading. */
//        Serial.println(distance);
//        digitalWrite(LEDPin, LOW);
//
//        /*
//         * Here we should send a message to our server
//         * to ket it know that someone passed through the door.
//         */
//        UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
//        UDP.write("Someone passed.");
//        UDP.endPacket();
//    }
//    
//    //Delay 50ms before next reading.
//    delay(50);
}
