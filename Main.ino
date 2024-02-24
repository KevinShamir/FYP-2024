/*
!! REMINDERS !! - Check all CONFIRMED?

1) Set default dirPin to clockwise and ensure dirPin in carrosselReset (x2) is Anticlockwise and alternate 
2) Amend U.S distance to glass slide
*/

/*
To amend for debugging
1) All the MotorSteps
2) All the reset for the 4 axes and at the bottom of the code (for the second magazine)
3) distanceScan = ultrasonicScan();
4) fanTiming

To Review
1) CONFIRMED? - values need to be confirmed
2) CHANGE? - values need to be changed
3) UNCOMMENT = values need to be uncommented

*/


// Libraries
#include <SoftwareSerial.h>
#include <Servo.h>  //Used to gripper


// Declare pins
int rxPin = 4;                    //tx connect rx//original is 3
int txPin = 2;                    // rx connect tx
int homeSwitchLongitudinal = 29;  //longitudinal arm limit switch
int homeSwitchLateral = 23;       //Lateral arm limit switch
int homeSwitchVertical = 28;      //Vertical arm limit switch



// Motor Pins
const int sigCarro = 5;  //signal pin for carroServo

const int pul3 = 7;  //Motor 3 Lateral (Left and Right)
const int dir3 = 8;

const int pul2 = 9;  //Motor 2 Longitudinal (Front and Back)
const int dir2 = 10;

const int pul4 = 11;  //Motor 4 Vertical (Up and Down)
const int dir4 = 12;

const int sigFan = 13;    //signal pin for PWM Fan
const int sigServo = 25;  //Signal pin for gripperServo

const int trigPin = 26;  //Trigger pin for U.S
const int echoPin = 27;  //Echo pin for U.S



//Servo motor initialisation

Servo gripperServo;  //Create servo object to control to gripperServo
const int servoMin = 500;
const int servoMax = 2540;
const int forceClose = 180;            //Used to force close the gripper
const int gripperOpen = 140;           //need amending
const int gripperClose = forceClose;  //closing value is going to be the same value as forceClose

Servo carroServo;            //create servo object to control carroServo
int carroInput = 180;  // CONFIRMED?
int carroFan = 100;     // CONFIRMED?
int carroPickup = 7;   // CONFIRMED?


//Coordinates Initialisation
float coordinate1;           //lateral -- distance from barcode scanner to magazine
float coordinate2;           //vertical-- distance from barcode scanner to magazine
float coordinate3;           //longitudinal-- distance from barcode scanner to magazine
char coordinateMessage[20];  //maximum size of message


// MotorSteps initialisation -- 1 Revolution is 200 steps
//const int fan = 50;                           // steps to rotate to fan from input area 90 degrees CONFIRMED?
//const int pickup = 50;                        //steps to rotate from fan to pickup area 90 degrees CONFIRMED?
const int lateralPickup = 800;                 //Steps to move lateral base from LIMIT SWITCH LATERAL to PICKUP POINT CONFIRMED?
const int verticalPickup = 2870;                //Steps to move vertical base from LIMIT SWITCH LATERAL to PICKUP POINT CONFIRMED?
const int longitudinalPickup = 700;            //Steps to move the longitudinal base from LIMIT SWITCH LATERAL to magazine CONFIRMED?
const int longitudinalScanIntermediate = 700;  //Steps to move the longitudinal base from magazine to an intermediate pickup, before QR Scanning CONFIRMED?
const int longitudinalScan = 0;              // Steps to move the longitudinal base from intermediate to scanning area (Both barcode and U.S) CONFIRMED?
const int lateralScan = 2600;                   //Steps to move the lateral base from pickup to QR Scanning CONFIRMED?
const int verticalScan = 2870;                  //Steps to move the vertical base from pickup to QR Scanning CONFIRMED?

//MotorSpeed initialisation
const int lateralSpeed = 800;          //delay value in microseconds CONFIRMED? -YES
const int verticalSpeed = 500;         //delay value in microseconds CONFIRMED? - YES
const int longitudinalSpeed = 500;  //delay value in microseconds CONFIRMED?

//U.S Variables
long duration;
int distance;
int distanceUsToSlide = 15;  //minimum distance recorded if glass slide is placed under U.S CONFIRMED?
long distanceScan;           // Used to store distance value

//Others
const int fanTiming = 15000;          //1 seconds fan duration, in milli seconds CONFIRMED?
int glassSlideCount = 0;             // Current number of glass slides in the magazine
const int glassSlideMax = 30;        // Max number of glass slides in one magazine
const int glassSlideStepInput = 50;  //Number of motor steps between each glass slide in the magazine input CONFIRMED?
const int scanDuration = 5000;       //max period of 5 seconds for barcode scanner to read QR Code once it is placed under the scanner CONFIRMED?
unsigned long start, finished, elapsed;
char input;
int firstFilled = 0;    //logic to indicate the first row of a magazine that contains a glass slide
int magazineBreak = 0;  //logic to determine the need to exit the while loop of 1 magazine
int magazine_num;
int magazineCount;  //global count of number of magazines used, init value is 0



//SoftwareSerial mySerial(rxPin, txPin);  // RX, TX for QR Code Scanner

//Function to read U.S readings
int ultrasonicScan() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  //Sets the trigPin on HIgh State for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sounds wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  //Calculating the distance
  distance = duration * 0.034 / 2;
  return distance;
}


// Functions to reset
void longitudinalReset() {
  // since the location of limit switch is already known, the direction of movement towards limit switch is also known
  //digitalWrite(dirPin,LOW);
  digitalWrite(dir2, HIGH);  // direction control
  int state = 1;
  while (state) {
    digitalWrite(pul2, HIGH);
    delayMicroseconds(500);
    digitalWrite(pul2, LOW);
    delayMicroseconds(500);
    if (digitalRead(homeSwitchLongitudinal) == HIGH) {
      state = 0;                //stop moving once switch is hit
      digitalWrite(dir2, LOW);  //Set motor rotating direction to clockwise default
                                //digitalWrite(dirPin,HIGH);
    }
  }
}

void lateralReset() {
  // since the location of limit switch is already known, the direction of movement towards limit switch is also known
  //digitalWrite(dirPin,LOW);
  digitalWrite(dir3, HIGH);  // move towards the limit switch
  int state = 1;
  while (state) {
    digitalWrite(pul3, HIGH);
    delayMicroseconds(800);
    digitalWrite(pul3, LOW);
    delayMicroseconds(800);
    if (digitalRead(homeSwitchLateral) == HIGH) {
      state = 0;                //stop moving once switch is hit
      digitalWrite(dir3, LOW);  //Set motor rotating direction to clockwise default
                                //digitalWrite(dirPin,HIGH);
    }
  }
}

void verticalReset() {
  // since the location of limit switch is already known, the direction of movement towards limit switch is also known
  //digitalWrite(dirPin,LOW);
  digitalWrite(dir4, LOW);
  int state = 1;
  while (state) {
    digitalWrite(pul4, HIGH);
    delayMicroseconds(400);
    digitalWrite(pul4, LOW);
    delayMicroseconds(400);
    if (digitalRead(homeSwitchVertical) == HIGH) {
      state = 0;                 //stop moving once switch is hit
      digitalWrite(dir4, HIGH);  //Set motor rotating direction to clockwise default
                                 //digitalWrite(dirPin,HIGH);
    }
  }
}

//Function to rotate the motor by a specific number of steps
void motorStep(int Steps, int pulPin, int dirPin, int direction, int Speed) {
  digitalWrite(dirPin, direction);
  for (int x = 0; x < Steps; x++) {

    digitalWrite(pulPin, HIGH);

    delayMicroseconds(Speed);

    digitalWrite(pulPin, LOW);

    delayMicroseconds(Speed);
  }
}

//Function to rotate input carrossel
void carro(int angle) {
  if (angle >= carroServo.read()) {
    for (int pos = carroServo.read(); pos <= angle ; pos += 1) {
      carroServo.write(pos);
      delayMicroseconds(7000);
    }
  } else {
    for (int pos = carroServo.read(); pos >= angle; pos -= 1) {
      carroServo.write(pos);
      delayMicroseconds(7000);
    }
  }
}

//Function to activate the fan for a specific period
void activateFan(int duration) {
  digitalWrite(sigFan, HIGH);
  delay(duration);  //duration to turn on the fan in milliseconds
  digitalWrite(sigFan, LOW);
}

//Function to control stepper motor for gripper
void gripper(float angle) {
  gripperServo.write(angle);
  delay(1000);
}


void setup() {
  //pinMode(homeSwitchCarrossel,INPUT_PULLUP); //limit switch setup
  pinMode(homeSwitchLongitudinal, INPUT_PULLUP);  //limit switch setup
  pinMode(homeSwitchLateral, INPUT_PULLUP);       //limit switch setup
  pinMode(homeSwitchVertical, INPUT_PULLUP);      //limit switch setup

  pinMode(pul2, OUTPUT);
  pinMode(dir2, OUTPUT);
  digitalWrite(dir2, LOW);  // Enables the motor to move in a particular direction

  pinMode(pul3, OUTPUT);
  pinMode(dir3, OUTPUT);
  digitalWrite(dir3, LOW);  // Enables the motor to move in a particular direction

  pinMode(pul4, OUTPUT);
  pinMode(dir4, OUTPUT);
  digitalWrite(dir4, LOW);  // Enables the motor to move in a particular direction

  //U.S
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   //Sets the echoPin as an Input
  //Other pins
  pinMode(sigFan, OUTPUT);  //PWM Fan logic control
  gripperServo.attach(sigServo, servoMin, servoMax);
  gripperServo.write(forceClose);  //To force close the gripper
  carroServo.attach(sigCarro);
  carroServo.write(carroPickup);  // Starting position @ input region
  delay(10);


  Serial.begin(9600);
  Serial1.begin(9600);  // set the data rate for the SoftwareSerial port
  while (!Serial) {}
}

void loop() {
  if (Serial.available() > 0) {
    String message = Serial.readStringUntil('\n');

    if (message.equals("Start")) {
      magazineCount=0;//Reset magazineCount
      delay(5);                    //short delay
      Serial.println("Received");  // Send Received message to Raspi

      while (1) {
        if (Serial.available() > 0) {
          String message = Serial.readStringUntil('\n');
          magazine_num = message.toInt();  //Convert message to integer. To be used in future to control how many rotations based on magazine input
          if (magazine_num > 0 && magazine_num == int(magazine_num)) {
            //Serial.println(magazine_num); //for debugging
            //Serial.print(magazine_num+1); //debugging
            carro(carroInput);
            delay(1000);  //short delay of 1 second
            longitudinalReset(); //Reset longitudinal arm to its limit switch position - UNCOMMENT
            verticalReset();// Reset vertical arm to its limit switch position - UNCOMMENT
            lateralReset(); //Reset lateral arm to its limit switch position - UNCOMMENT
            
            //carrosselReset(); //Reset input magazine towards limit switch. No check with laser sensor needed as check is done with limit switch - OBSOLETE
            Serial.println("Received");  // Send Received message to Raspi

            while (1) {
              if (Serial.available() > 0) {
                String message = Serial.readStringUntil('\n');
                //Serial.println(message); //debugging
                if (message.equals("Inserted")) {
                  magazineCount++;  //Increase magazineCount by 1
                  // Rotating carrossel from input to fan
                  firstFilled = 0;      //reset logic to initial condition
                  magazineBreak = 0;    //reset logic to initial condition
                  glassSlideCount = 0;  //reset glass slide Count
                  delay(5);                    //short delay
                  Serial.println("Received");  // Send Received message to Raspi
                  delay(3000);                 //Delay of 3 seconds before commencing rotation
                  carro(carroFan);
                  //Serial.println("Fan is activated"); //debug
                  delay(1000);  // Delay of 1 second before activating the fan

                  // Activating fan and rotating carrossel to pickup area
                  activateFan(fanTiming);
                  delay(1000);  //Delay of 1 second before rotating to pickup area
                  carro(carroPickup);
                  delay(5);  //Short delay before activating Lateral system

                  

                  //Bring Lateral, vertical slides to pickup place
                  motorStep(lateralPickup, pul3, dir3, 0, lateralSpeed);  // Reviewed
                  delay(5);                                               // Short delay before activating vertical system
                  motorStep(verticalPickup, pul4, dir4, 1, verticalSpeed);  // Reviewed
                  delay(1000);                                              //delay of 1 second before opening gripper mouth
                  //Serial.println("Im at the pickup"); //debug

                  while (glassSlideCount < 3) {  //CHANGE? to 30
                    //Open gripper and move to magazine and close gripper
                    //Serial.println("Im in the glass slide count while loop"); //debug
                    gripper(gripperOpen);                                             //open Gripper mouth
                    delay(5);                                                         //short delay
                    motorStep(longitudinalPickup, pul2, dir2, 0, longitudinalSpeed);  //Reviewed
                    delay(1000);                                                      //1s delay before closing gripper mouth
                    gripper(gripperClose);                                            //Close Gripper Mouth
                    glassSlideCount++;                                                //Increase glassSlideCount by 1
                    //Serial.println("Ive added glass slide count"); //debug

                    //Bring longitudinal, lateral and vertical to scanning intermediate point. Intermediate point (logitudinally) is in between magazine @ pickup and gripper position before moving logitudinally to the magazine
                    motorStep(longitudinalScanIntermediate, pul2, dir2, 1, longitudinalSpeed);  //Reviewed
                    delay(5); //short delay before activating lateral system
                    motorStep(verticalScan, pul4, dir4, 0, verticalSpeed);                      //Reviewed
                    delay(5);                                                                   //Short delay before activating Lateral system                                                                  
                    motorStep(lateralScan, pul3, dir3, 0, lateralSpeed);                        //Reviewed
                    delay(5);                                                                   //Short delay before activating vertical system
                    //Serial.println("I am at the Interim point"); //debug
                    //delay(5000); //debug

                    //Bring longitudinal arm to scanning point, scan using ultrasonic sensor
                    motorStep(longitudinalScan, pul2, dir2, 0, longitudinalSpeed);  //Reviewed
                    int summation = 0;                                              // placeholder to store summation value
                    for (int i = 0; i < 6; i++) {
                      distanceScan = 1;  // ultrasonicScan(); //Record distance glass slide is from ultrasonic sensor over 6 iterations. ignore the value from first iterations - CHANGE?
                      if (i > 0) {
                        summation = summation + distanceScan;
                      }
                    }
                    distanceScan = (summation / 5);  //assign summation value to distanceScan
                    summation = 0;
                    //Serial.println("I have completed US Scanning"); //debug

                    //If glass slide is not detected, move on to the next row of the magazine
                    while (distanceScan > distanceUsToSlide) {
                      if (firstFilled) {
                        magazineBreak = 1;  //if an upper magazine has already been detected, this is an empty glass slide
                        break;              //exit out of the while loop to exit of the magazine loop
                      }

                      int verticalDeduction = glassSlideCount * glassSlideStepInput;
                      motorStep(longitudinalScan, pul2, dir2, 1, longitudinalSpeed);              //Rotate longitudinal motor anticlockwise. !! Remember to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(5);                                                                   //Short delay before activating longitudinal system
                      motorStep(verticalScan + verticalDeduction, pul4, dir4, 1, verticalSpeed);  //Rotate vertical motor anticlockwise. !!Remember to amend the steps and direction. Direction needs to be validated
                      delay(5);                                                                   //Short delay before activating longitudinal system
                      motorStep(lateralScan, pul3, dir3, 0, lateralSpeed);                        //Rotate lateral motor clockwise. !! Reminder to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(5);                                                                   //short delay

                      //Open gripper and move to magazine and close gripper
                      gripper(gripperOpen);                                                       //open Gripper mouth
                      delay(5);                                                                   //short delay
                      motorStep(longitudinalScanIntermediate, pul2, dir2, 0, longitudinalSpeed);  //Rotate longitudinal motor clockwise. !! Remember to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(1000);                                                                //1s delay before closing gripper mouth
                      gripper(gripperClose);                                                      //Close Gripper Mouth
                      glassSlideCount++;                                                          //increase glassSlideCount by 1


                      //Bring longitudinal, lateral and vertical to scanning intermediate point,
                      motorStep(longitudinalScanIntermediate, pul2, dir2, 1, longitudinalSpeed);  //Rotate longitudinal motor anticlockwise. !! Remember to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(5);                                                                   //Short delay before activating lateral system
                      motorStep(lateralScan, pul3, dir3, 1, lateralSpeed);                        //Rotate lateral motor anticlockwise. !! Reminder to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(5);                                                                   //Short delay before activating vertical system
                      motorStep(verticalScan + verticalDeduction, pul4, dir4, 0, verticalSpeed);  //Rotate vertical motor clockwise. !!Remember to amend the steps and direction. Direction needs to be validated
                      delay(5);                                                                   //Short delay before activating longitudinal system

                      //Bring longitudinal to scanning point, scan using ultrasonic sensor
                      motorStep(longitudinalScan, pul2, dir2, 0, longitudinalSpeed);  //Rotate longitudinal motor clockwise. !! Remember to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(5);                                                       //Short delay before measuring distance
                      int summation = 0;                                              // placeholder to store summation value
                      for (int i = 0; i < 6; i++) {
                        distanceScan = 1;  //ultrasonicScan(); //Record distance glass slide is from ultrasonic sensor over 6 iterations. ignore the value from first iterations
                        if (i > 0) {
                          summation = summation + distanceScan;
                        }
                      }
                      distanceScan = (summation / 5);  //assign summation value to distanceScan
                      summation = 0;
                    }

                    firstFilled = 1;  //setting the variable to true the moment a physical slide is detected at the top
                    if (magazineBreak) {
                      break;  //exit of while loop for a magazine
                    }

                    //Barcode Scanning- Activate scanner if U.S distance is within range
                    bool condition = true;
                    start = millis();  //start arduino timer
                    while (condition) {
                      finished = millis();
                      elapsed = finished - start;
                      //Serial.println(elapsed); //debug
                      if (1) {  //elapsed < scanDuration){ //elapsed < scanDuration TO change - UPDATE?
                        //Serial.println("I am in elapse < scanDuration"); //debug
                        if (Serial1.available()) {
                          while (Serial1.available()) {
                            input = Serial1.read();
                            //Serial.print(input); //debug
                            bool logic = (input != "");  //check if qr code value is not empty. True if not empty
                            if (logic) {
                              Serial.print(input);  //Send message to Raspi if value is not empty
                              //Serial.println("I have sent the NB Number"); //debug
                              delay(5);
                              condition = false;
                              //break; //break out of while loop
                            } else {                   // Send Faulty message to Raspi
                              Serial.print("Faulty");  //Send faulty message to Raspi if it is an empty barcode
                              condition = false;
                              //Serial.println("Empty barcode detected"); //debug
                              break;  //break out of while loop
                            }
                          }
                        }
                      } else {
                        condition = false;
                        Serial.print("Faulty");  //Send faulty message to Raspi if QR scanner is taking too long to scan
                        //Serial.println("Faulty barcode detected"); //debug
                      }
                    }

                    //For both faulty or working barcodes, coordinates will be sent over. A while loop is sent up and only broken till the coordinates are sent over
                    while (1) {
                      //Serial.println("I am now waiting for the coord");// debug
                      if (Serial.available() > 0) {
                        String message = Serial.readStringUntil('\n');
                        //Serial.println(message); //debug
                        message.toCharArray(coordinateMessage, sizeof(coordinateMessage));
                        char *ptr;  // declare a pointer
                        ptr = strtok(coordinateMessage, ",");
                        coordinate1 = atof(ptr);
                        coordinate2 = atof(strtok(NULL, " , "));
                        coordinate3 = atof(strtok(NULL, " , "));
                        delay(5);                    //short delay
                        Serial.println("Received");  // Send Received message to Raspi
                        break;                       //break from while loop
                      }
                    }

                    //Bring glass slide through longitudinal, lateral and vertical to dedicatd magazine slot
                    motorStep(longitudinalScanIntermediate, pul2, dir2, 1, longitudinalSpeed);  //Rotate longitudinal motor anticlockwise. !! Remember to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                    delay(5);                                                                   //short delay before activating lateral system
                    motorStep(coordinate1, pul3, dir3, 1, lateralSpeed);                        //Rotate lateral motor anticlockwise. !! Reminder to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                    delay(5);                                                                   //Short delay before activating vertical system
                    motorStep(coordinate2, pul4, dir4, 1, verticalSpeed);                       //Rotate vertical motor anticlockwise. !!Remember to amend the steps and direction. Direction needs to be validated
                    delay(5);                                                                   //short delay
                    motorStep(coordinate3, pul2, dir2, 0, longitudinalSpeed);                   //Rotate longitudinal motor clockwise. !!Reminder to amend the steps and direction
                    delay(1000);                                                                //1s delay before closing gripper mouth
                    gripper(gripperOpen);                                                       //Open Gripper Mouth
                    delay(1000);                                                                //1s delay before moving back to pickup area
                    //Serial.println("I am at the output magazine slot");//debug


                    //Bring gripper to input magazine
                    motorStep(coordinate3 + (longitudinalPickup - longitudinalScanIntermediate), pul2, dir2, 1, longitudinalSpeed);  //Rotate longitudinal motor Anticlockwise. !!Reminder to amend the steps and direction. Change direction
                    delay(5);                                                                                                        //short delay
                    int verticalDeduction = glassSlideCount * glassSlideStepInput;
                    motorStep(abs(coordinate2 - verticalScan) - verticalDeduction, pul4, dir4, 0, verticalSpeed);  //Rotate vertical motor anticlockwise. !!Remember to amend the steps and direction. Change direction
                    delay(5);                                                                                      //short delay
                    motorStep(coordinate1 + lateralScan, pul3, dir3, 0, lateralSpeed);                             //Rotate lateral motor clockwise. !! Reminder to amend the steps and direction. Change direction
                    delay(5);                                                                                      //Short delay before activating vertical system
                    //Serial.println("I am at the end of the loop for one slide"); //debug
                  }
                  if (magazine_num == 2 && magazineCount < 2) {
                    delay(1000);  //1 second delay before activating resets
                    //longitudinalReset(); //Reset longitudinal arm to its limit switch position
                    //verticalReset();// Reset vertical arm to its limit switch position
                    //lateralReset(); //Reset lateral arm to its limit switch position
                    carro(carroInput);  // rotate carrosel back to input position
                    Serial.print("Magazine");      // message sent to Raspi to indicate the end of the first magazine. Now awaiting "Inserted message"
                  } else {
                    //Serial.println("I am breaking out of inserted message while loop"); //debug
                    break;  // break out of the while loop for the inserted message
                  }
                }
              }
            }
            //Serial.println("I am breaking out of magazine_num while loop"); //debug
            break;  //break out of the while loop for magazine_num
          }
        }
      }
    }
    Serial.println("Finished");  //send message sent to ardRaspi to indicate a completed ,magazine
  }
}
