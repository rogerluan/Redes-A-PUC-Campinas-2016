#include <SPI.h>        
#include <Ethernet.h>
#include <EthernetUdp.h>

#define echoPin 7 // Echo Pin
#define triggerPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

unsigned int localPort = 8888;      // local port to listen on

EthernetUDP UDP; // An EthernetUDP instance to let us send and receive packets over UDP
int minimumRange = 0; // Minimum distance of the sensor
int maximumRange = 90; // Maximum width of the door
long duration, distance; // Duration used to calculate distance

void setup() {
    //setup sonar sensor and LED
    Serial.begin (9600);
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(LEDPin, OUTPUT); // Use LED indicator (will only be used to debug)

    //setup ethernet and UDP:
    Ethernet.begin(mac,ip);
    UDP.begin(localPort);
}

void loop() {
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
    
    if (distance >= maximumRange || distance <= minimumRange) { //if the range is out of the bounds
        /*
         * This shouldn't do anything. Just log some error (or light a LED).
         */
        Serial.println("-1");
        digitalWrite(LEDPin, HIGH);
    } else { //if the range is within the defined bounds
        /* Send the distance to the computer using Serial protocol, and
         turn LED OFF to indicate successful reading. */
        Serial.println(distance);
        digitalWrite(LEDPin, LOW);

        /*
         * Here we should send a message to our server
         * to ket it know that someone passed through the door.
         */
        UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
        UDP.write("Someone passed.");
        UDP.endPacket();
    }
    
    //Delay 50ms before next reading.
    delay(50);
}
