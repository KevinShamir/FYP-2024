// defines pins numbers

const int pulPin = 8;

const int dirPin = 9;

long number;

// encoder definition

#include "AS5600.h"
#include "Wire.h"

AS5600 as5600;   //  use default Wire


void setup() {
  Serial.begin(9600);

  // put your setup code here, to run once:
  pinMode(pulPin,OUTPUT);

  pinMode(dirPin,OUTPUT);

  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction

  //encoder
  Wire.begin();

  as5600.begin(2);  //  set direction pin.
  as5600.setDirection(AS5600_CLOCK_WISE);  //  default, just be explicit.

  //Serial.println(as5600.getAddress());

  //  as5600.setAddress(0x40);  //  AS5600L only

  int b = as5600.isConnected();
  //Serial.print("Connect: ");
  //Serial.println(b);

  delay(1000);
  Serial.println("Please enter the number of Steps to be made: ");

  
}

void motorStep( long MAX){


   for(long x = 0; x < MAX; x++) {

        digitalWrite(pulPin,HIGH);

        delayMicroseconds(500);

        digitalWrite(pulPin,LOW);

        delayMicroseconds(500);
        as5600.getCumulativePosition();
        as5600.getRevolutions();

      }

}

void loop() {
  if (Serial.available() > 0) { // Check if there is data available to read
    as5600.getCumulativePosition();
    number = Serial.parseInt(); // Read the input number from the Serial Monitor
    Serial.print("You have entered:");
    Serial.print("\t");
    Serial.println(number);
    motorStep(number);
    Serial.print("Number of steps recorded by encoder:");
    Serial.println("\t");
    //Serial.println(as5600.getRevolutions());
    Serial.println(as5600.getCumulativePosition()/20.48);
    as5600.resetPosition();
    delay(5);
    while (Serial.available() > 0) {
      char c = Serial.read(); // Read and discard remaining characters in the buffer
    }
  }
}

