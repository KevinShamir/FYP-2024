/***********************************************************
File name: 02_activeBuzzer.ino
Description: Arduino uno Continuous beeps control buzzer.
Website: www.adeept.com
E-mail: support@adeept.com
Author: Tom
Date: 2015/05/02 
***********************************************************/
int piezoPin=11; //definition digital 8 pins as pin to control the buzzer

void beep(unsigned int frequency, unsigned long beepDuration, unsigned long pauseDuration, unsigned long totalDuration) {
  unsigned long startTime = millis(); // Get the current time
  unsigned long endTime = startTime + totalDuration; // Calculate the end time

  while (millis() < endTime) {
    // Calculate the period of the beep in microseconds
    unsigned long period = 1000000UL / frequency;

    // Calculate the number of cycles for the given duration
    unsigned long cycles = (beepDuration * 1000UL) / (period * 2);

    // Generate the beep by toggling the piezo pin
    for (unsigned long i = 0; i < cycles; i++) {
      digitalWrite(piezoPin, HIGH);
      delayMicroseconds(period);
      digitalWrite(piezoPin, LOW);
      delayMicroseconds(period);
    }

    // Pause between consecutive beeps
    delay(pauseDuration);
  }
}
void setup()
{
    pinMode(piezoPin,OUTPUT); //Set digital 8 port mode, the OUTPUT for the output
}
void loop()
{  
    beep(2000, 100, 100, 10000); // Beep at 2000Hz for 50 milliseconds, with a 50 milliseconds pause, and a total duration of 10 seconds
    delay(10000);                   //Set the delay time，2000ms
}
