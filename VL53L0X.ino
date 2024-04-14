
#include "Adafruit_VL53L0X.h"
#include "MedianFilter.h"

MedianFilter test(10,0);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

const int numReadings = 20; // Number of readings to average

void setup() {
  Serial.begin(9600);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  Serial.println("Adafruit VL53L0X test.");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  // power
  Serial.println(F("VL53L0X API Continuous Ranging example\n\n"));

  // start continuous ranging
  lox.startRangeContinuous();
}

void loop() {
  
  if (lox.isRangeComplete()) {
    //Serial.print("Distance in mm: ");
    Serial.println(lox.readRange());
    //int val = lox.readRange();
    //test.in(val);
   // val=test.out();
    //Serial.println(val);
    delay(66);
  
  }
  /*
  
  int total = 0;
  for (int i=0; i<numReadings; i++){
    int val = lox.readRange();
    test.in(val);
    val=test.out();
    //int val = lox.readRange();
    total += val;
    delayMicroseconds(10);
  }
  int average = total/numReadings;
  Serial.println(average);

  */
}
