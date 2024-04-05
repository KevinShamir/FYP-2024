#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 2); // RX, TX

//Ensure TTL 232 QR CODE IS SCAN FOR PRODUCT
char input;
 
void setup()
{
  Serial.begin(9600);  
  mySerial.begin(9600); // set the data rate for the SoftwareSerial port
}
 
void loop()
{
  if (mySerial.available()) // Check if there is Incoming Data in the Serial Buffer.
  {
    while (mySerial.available()) // Keep reading Byte by Byte from the Buffer till the Buffer is empty
    {
      input = mySerial.read(); // Read 1 Byte of data and store it in a character variable
      Serial.print(input); // Print the Byte
      delay(5); // A small delay
      Serial.println();
    }
    bool logic=(input!="");
    Serial.print(logic);
    Serial.println();
  }
}
