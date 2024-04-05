#include <Servo.h>
Servo myservo;
void setup() {
  Serial.begin(9600);
  myservo.attach(5,500,2540);
  myservo.write(50);
}

void loop() {
  int val;  //creating a val var
  while (Serial.available() > 0) {
    val = Serial.parseInt();
    if (val > 0) {
      Serial.println(myservo.read());
      if (val >= myservo.read()) {
        for (int pos = myservo.read(); pos <= val; pos += 1) {
          myservo.write(pos);
          delayMicroseconds(6000);
          Serial.println(val);
          //myservo.write(val);
        }
      } else {
        for (int pos = myservo.read(); pos >= val; pos -= 1) {
          myservo.write(pos);
          delayMicroseconds(6000);
          Serial.println(val);
          //myservo.write(val);
        }
      }
    }
  }
}
