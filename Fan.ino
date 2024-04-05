/*
 * 
 * Only need to use +12v, GND and PWM pin of fan. Signal pin dont have to be used
 */

int sig_fan=13;
void setup() {
  // put your setup code here, to run once:
  pinMode(sig_fan,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(sig_fan,HIGH);
  delay(10000); //duration to turn on the fan in milliseconds 
  digitalWrite(sig_fan,LOW);
  delay(10000);
  
}
