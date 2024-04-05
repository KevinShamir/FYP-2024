/*
Authors: 
Shamir Kevin
Gokul Selveraj

*/


// Libraries
#include <SoftwareSerial.h> //Used for serial communication
#include <Servo.h>  //Used for gripper and carrossel


// Digtal pins - limit switches & rx/tx
int rxPin = 4;                    //tx connect rx
int txPin = 2;                    // rx connect tx
int homeSwitchLongitudinal = 40;  //longitudinal arm limit switch
int homeSwitchLateral = 23;       //Lateral arm limit switch
int homeSwitchVertical = 31;      //Vertical arm limit switch



// Digital Pins
const int sigCarro = 5;  //signal pin for carroServo

const int pul3 = 2;  //Motor 3 Lateral (Left and Right)
const int dir3 = 8;

const int pul2 = 14;  //Motor 2 Longitudinal (Front and Back)
const int dir2 = 21;

const int pul4 = 45;  //Motor 4 Vertical (Up and Down)
const int dir4 = 10;

const int pwmFan = 7;    //signal pin for PWM Fan
const int sigServo = 33;  //Signal pin for gripperServo

const int trigPin = 26;  //Trigger pin for U.S
const int echoPin = 27;  //Echo pin for U.S

const int piezoPin = 11; //buzzerPin



//Servo motor initialisation

Servo gripperServo;  //Create servo object to control to gripperServo
const int servoMin = 500;
const int servoMax = 2540;
const int forceClose = 80;            //Used to force close the gripper
const int gripperOpen = 55;           //Used to open gripper at input magazine
const int gripperOpenOutput = 67;     //Used to open gripper at output magazine
const int gripperOutputClose = 68;    // used to slightly close gripper at output magazine
const int gripperClose = 90;  //Used to close gripper at input magazine

Servo carroServo;            //create servo object to control carroServo
int carroInput = 180;  // Used to rotate carrossel to input area
int carroFan = 95;     // Used to rotate carrossel to input - fan
int carroPickup = 7;   // Used to rotatae carrossel to input - pickup

//Coordinates Initialisation
long coordinate1;           //lateral -- distance from barcode scanner to magazine
long coordinate2;           //vertical-- distance from barcode scanner to magazine
long coordinate3;           //longitudinal-- distance from barcode scanner to magazine
char coordinateMessage[100];  //maximum size of message


// MotorSteps initialisation -- 1 Revolution is 200 steps
const long lateralPickup = 1100;                 //Steps to move lateral base from LIMIT SWITCH LATERAL to PICKUP POINT
const long verticalPickup = 99300;                //Steps to move vertical base from LIMIT SWITCH VERTICAL to PICKUP POINT
const long longitudinalPickup = 54500;            //Steps to move the longitudinal base from LIMIT SWITCH LONGITUDINAL to PICKUP POINT
const long longitudinalScanIntermediate = 54500;  //Steps to move the longitudinal base from PICKUP POINT to INTERMEDIATE POINT
const long longitudinalScan = 16000;              // Steps to move the longitudinal base from PICKUP POINT to SCANNING POINT
const long lateralScan = 2850;                   //Steps to move the lateral base from PICKUP POINT to SCANNING POINT
const long outputLongitudinal = 1600;             // Steps to move longitudianl base from OUTPUT MAGAZINE to SLIGHTLY BACKWARDS

//MotorSpeed initialisation
const int lateralSpeed = 1200;          //delay value in microseconds 
const int verticalSpeed = 40;         //delay value in microseconds 
const int longitudinalSpeed = 40;  //delay value in microseconds 
const int longitudinalSpeedOutput = 20; //delay value in microseconds

//U.S Variables
long duration;
int distance;
int distanceUsToSlide = 14;  //minimum distance recorded if glass slide is placed under U.S
long distanceScan;           // Used to store distance value

//Others
const int fanTiming = 30000;          //30 seconds fan duration, in milli seconds 
long glassSlideCount = 0;             // Current number of glass slides in the magazine
const long glassSlideMax = 30;        // Max number of glass slides in one magazine
const long glassSlideStepInput = 3175;  //Number of motor steps between each glass slide in the magazine input
const int scanDuration = 10000;       //max period of 10 seconds for barcode scanner to read QR Code once it is placed under the scanner
unsigned long start, finished, elapsed; //Used for measuring time
char input; // variable to store NB number scanned
int firstFilled = 0;    //logic to indicate the first row of a magazine that contains a glass slide
int magazineBreak = 0;  //logic to determine the need to exit the while loop of 1 magazine
int magazine_num;   //Number of magazines user is inserting
int magazineCount;  //global count of number of magazines used, init value is 0

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
  delay(10);
  digitalWrite(dir2, LOW);  // direction control
  delay(10);
  int state = 1;
  while (state) {
    digitalWrite(pul2, HIGH);
    delayMicroseconds(longitudinalSpeed);
    digitalWrite(pul2, LOW);
    delayMicroseconds(longitudinalSpeed);
    if (digitalRead(homeSwitchLongitudinal) == LOW) {
      state = 0;                //stop moving once switch is hit
      digitalWrite(dir2, HIGH);  //Set motor rotating direction to clockwise default
    }
  }
}

void lateralReset() {
  delay(10);
  digitalWrite(dir3, HIGH);  // move towards the limit switch
  delay(10);
  int state = 1;
  while (state) {
    digitalWrite(pul3, HIGH);
    delayMicroseconds(lateralSpeed);
    digitalWrite(pul3, LOW);
    delayMicroseconds(lateralSpeed);
    if (digitalRead(homeSwitchLateral) == LOW) {
      state = 0;                //stop moving once switch is hit
      digitalWrite(dir3, LOW);  //Set motor rotating direction to clockwise default
    }
  }
}

void verticalReset() {
  delay(10);
  digitalWrite(dir4, LOW);
  delay(10);
  int state = 1;
  while (state) {
    digitalWrite(pul4, HIGH);
    delayMicroseconds(verticalSpeed);
    digitalWrite(pul4, LOW);
    delayMicroseconds(verticalSpeed);
    if (digitalRead(homeSwitchVertical) == LOW) {
      state = 0;                 //stop moving once switch is hit
      digitalWrite(dir4, HIGH);  //Set motor rotating direction to clockwise default
    }
  }
}

//Motor Function - rotate motor by number of steps in a certain direction at a certain speed
void motorStep(long Steps, int pulPin, int dirPin, int direction, int Speed) {
  delay(10);
  digitalWrite(dirPin, direction);
  delay(10);
  for (long x = 0; x < Steps; x++) {

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
      delayMicroseconds(7500);
    }
  } else {
    for (int pos = carroServo.read(); pos >= angle; pos -= 1) {
      carroServo.write(pos);
      delayMicroseconds(7500);
    }
  }
}

//Function to activate the fan for a specific period
void activateFan(int duration) {
  digitalWrite(pwmFan, HIGH);
  delay(duration);  //duration to turn on the fan in milliseconds
  digitalWrite(pwmFan, LOW);
}

//Function to control servo motor for gripper
void gripper(float angle) {
  gripperServo.write(angle);
  delay(1000);
}

// Function to generate a beep on the piezo buzzer
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


void setup() {
  pinMode(homeSwitchLongitudinal, INPUT);  //limit switch setup
  pinMode(homeSwitchLateral, INPUT);       //limit switch setup
  pinMode(homeSwitchVertical, INPUT);      //limit switch setup

  pinMode(pul2, OUTPUT);
  pinMode(dir2, OUTPUT);
  digitalWrite(dir2, LOW);  // Enables the motor to move in a particular direction

  pinMode(pul3, OUTPUT);
  pinMode(dir3, OUTPUT);
  digitalWrite(dir3, LOW);  // Enables the motor to move in a particular direction

  pinMode(pul4, OUTPUT);
  pinMode(dir4, OUTPUT);
  digitalWrite(dir4, LOW);  // Enables the motor to move in a particular direction

  digitalWrite(piezoPin,OUTPUT); //Initialise Buzzer
  digitalWrite(piezoPin,LOW);

  //U.S
  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   //Sets the echoPin as an Input     
  //Other pins
  pinMode(pwmFan, OUTPUT);  //PWM Fan logic control
  digitalWrite(pwmFan,LOW); //set fan to off default
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
          magazine_num = message.toInt();  //Convert message to integer
          if (magazine_num > 0 && magazine_num == int(magazine_num)) {
            //Serial.println(magazine_num); //For debugging
            delay(1000);  //short delay of 1 second
            longitudinalReset(); //Reset longitudinal arm to its limit switch position
            verticalReset();// Reset vertical arm to its limit switch position
            lateralReset(); //Reset lateral arm to its limit switch position
            motorStep(lateralPickup+lateralScan, pul3, dir3, 0, lateralSpeed);  // Move lateral system to avoid hitting carrossel
            carro(carroInput);
            Serial.println("Received");  // Send Received message to Raspi

            while (1) {
              beep(2000, 100, 100, 10000); // Beep at 2000Hz for 50 milliseconds, with a 50 milliseconds pause, and a total duration of 10 seconds
              if (Serial.available() > 0) {
                String message = Serial.readStringUntil('\n');
                //Serial.println(message); //for debugging
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
                  //Serial.println("Fan is activated"); //For debugging
                  delay(1000);  // Delay of 1 second before activating the fan

                  // Activating fan and rotating carrossel to pickup area
                  activateFan(fanTiming);
                  delay(1000);  //Delay of 1 second before rotating to pickup area
                  carro(carroPickup);
                  delay(5);  //Short delay before activating Lateral system
                  lateralReset(); //Reset lateral arm to its limit switch position - UNCOMMENT

                  //Bring Lateral, vertical slides to pickup place
                  motorStep(lateralPickup, pul3, dir3, 0, lateralSpeed);  // Bring Lateral system to pickup area
                  delay(5);                                               // Short delay before activating vertical system
                  motorStep(verticalPickup, pul4, dir4, 1, verticalSpeed);  // Bring Vertical system to pickup area
                  //Serial.println("Im at the pickup"); //For debugging

                  while (glassSlideCount < glassSlideMax) {  //Keep track of total number of glass slides picked up
                    delay(1000);
                    //Serial.println("Im in the glass slide count while loop"); //For debugging
                    gripper(gripperOpen);                                             //open Gripper mouth
                    delay(5);                                                         //short delay
                    motorStep(longitudinalPickup, pul2, dir2, 1, longitudinalSpeed);  //Reviewed
                    delay(1000);                                                      //1s delay before closing gripper mouth
                    gripper(gripperClose);                                            //Close Gripper Mouth
                    long verticalDeduction = glassSlideCount * glassSlideStepInput;
                    glassSlideCount++;                                                //Increase glassSlideCount by 1
                    //Serial.println("Ive added glass slide count"); //For debugging

                    //Bring longitudinal, lateral and vertical to scanning intermediate point. Intermediate point (logitudinally) is in between magazine @ pickup and gripper position before moving logitudinally to the magazine
                    motorStep(longitudinalScanIntermediate, pul2, dir2, 0, longitudinalSpeed);  //Bring longitudinal system to intermediate point
                    delay(5); //short delay before activating vertical system
                    motorStep(verticalPickup-verticalDeduction,pul4, dir4, 0, verticalSpeed);
                    delay(5);                                                                   //Short delay before activating Lateral system                                                                  
                    motorStep(lateralScan, pul3, dir3, 0, lateralSpeed);                        //Reviewed
                    delay(5);                                                                   //Short delay before activating vertical system
                    //Serial.println("I am at the Interim point"); //For debugging

                    //Bring longitudinal arm to scanning point, scan using ultrasonic sensor
                    motorStep(longitudinalScan, pul2, dir2, 1, longitudinalSpeed);  //Reviewed
                    int summation = 0;                                              // placeholder to store summation value
                    for (int i = 0; i < 11; i++) {
                      distanceScan = ultrasonicScan(); //Record distance glass slide is from ultrasonic sensor over 6 iterations. ignore the value from first iterations - CHANGE?
                      if (i > 0) {
                        summation = summation + distanceScan;
                      }
                    }
                    distanceScan = (summation / 10);  //assign summation value to distanceScan
                    //Serial.println(distanceScan);
                    summation = 0;
                    //Serial.println("I have completed US Scanning"); //For debugging

                    while (distanceScan > distanceUsToSlide) { //If no glass slide deducted, enter the while loop
                      if (firstFilled) {  //If an no glass slide was detected, we have reached the last filled row
                        magazineBreak = 1;  //if an upper magazine has already been detected, this is an empty glass slide
                        break;              //exit out of the while loop to exit of the magazine loop
                      }

                      long verticalDeduction = glassSlideCount * glassSlideStepInput;
                      longitudinalReset(); //Reset back longitudinally
                      delay(5); //Short delay before activating vertical system
                      verticalReset(); //Reset back vertically
                      delay(5); //Short delay before activating lateral system
                      motorStep(lateralScan, pul3, dir3, 1, lateralSpeed); //Move laterally towards pickup point
                      delay(5);
                      motorStep(verticalPickup-verticalDeduction,pul4, dir4, 1, verticalSpeed);
                      delay(5);
                      gripper(forceClose);
                      delay(1000);  

                      gripper(gripperOpen);                                             //open Gripper mouth
                      delay(5);                                                         //short delay
                      motorStep(longitudinalPickup, pul2, dir2, 1, longitudinalSpeed);  //Reviewed
                      delay(1000);                                                      //1s delay before closing gripper mouth
                      gripper(gripperClose);                                            //Close Gripper Mouth
                      verticalDeduction = glassSlideCount * glassSlideStepInput;
                      glassSlideCount++;        


                      //Bring longitudinal, lateral and vertical to scanning intermediate point. Intermediate point (logitudinally) is in between magazine @ pickup and gripper position before moving logitudinally to the magazine
                      motorStep(longitudinalScanIntermediate, pul2, dir2, 0, longitudinalSpeed);  //Bring longitudinal system to intermediate point
                      delay(5); //short delay before activating vertical system
                      motorStep(verticalPickup-verticalDeduction,pul4, dir4, 0, verticalSpeed);
                      delay(5);                                                                   //Short delay before activating Lateral system                                                                  
                      motorStep(lateralScan, pul3, dir3, 0, lateralSpeed);                        //Reviewed
                      delay(5);                                                                   //Short delay before activating vertical system                                                                 //Short delay before activating longitudinal system

                      //Bring longitudinal to scanning point, scan using ultrasonic sensor
                      motorStep(longitudinalScan, pul2, dir2, 1, longitudinalSpeed);  //Rotate longitudinal motor clockwise. !! Remember to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                      delay(5);                                                       //Short delay before measuring distance
                      int summation = 0;                                              // placeholder to store summation value
                      for (int i = 0; i < 11; i++) {
                        distanceScan = ultrasonicScan(); //Record distance glass slide is from ultrasonic sensor over 10 iterations. ignore the value from first iterations
                        if (i > 0) {
                          summation = summation + distanceScan;
                        }
                      }
                      distanceScan = (summation / 10);  //assign summation value to distanceScan
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
                      //Serial.println(elapsed); //For debugging
                      if (elapsed < scanDuration) {  //check if duration within set limit
                        //Serial.println("I am in elapse < scanDuration"); //For debugging
                        if (Serial1.available()) {
                          while (Serial1.available()) {
                            input = Serial1.read();
                            //Serial.print(input); //For debugging
                            bool logic = (input != "");  //check if qr code value is not empty. True if not empty
                            if (logic) {
                              Serial.print(input);  //Send message to Raspi if value is not empty
                              //Serial.println("I have sent the NB Number"); // For debugging
                              delay(5);
                              condition = false;
                            } else {                   // Send Faulty message to Raspi
                              Serial.print("Faulty");  //Send faulty message to Raspi if it is an empty barcode
                              condition = false;
                              //Serial.println("Empty barcode detected"); //For debugging
                              break;  //break out of while loop
                            }
                          }
                        }
                      } 
                      else {
                        condition = false;
                        Serial.print("Faulty");  //Send faulty message to Raspi if QR scanner cant scan barcode but a glass slide is present
                        //Serial.println("Faulty barcode detected"); //For debugging
                      }
                    }

                    //For both faulty or working barcodes, coordinates will be sent over. A while loop is sent up and only broken till the coordinates are sent over
                    while (1) {
                      //Serial.println("I am now waiting for the coord");// For debugging
                      if (Serial.available() > 0) {
                        String message = Serial.readStringUntil('\n');
                        message.replace("(","");
                        message.replace(")","");
                        //Serial.println(message); // For debugging
                        message.toCharArray(coordinateMessage, sizeof(coordinateMessage));
                        char *ptr;  // declare a pointer
                        ptr = strtok(coordinateMessage, ",");
                        coordinate1 = atof(ptr);
                        coordinate2 = atof(strtok(NULL, " , "));
                        coordinate3 = atof(strtok(NULL, " , "));
                        delay(5);                    //short delay
                        //Serial.println("Received");  // For debugging
                        if(coordinate1!=0 && coordinate2 !=0 && coordinate3!=0){
                          break;                       //break from while loop
                        }
                      }
                    }

                    //Bring glass slide through longitudinal, lateral and vertical to dedicatd magazine slot
                    verticalReset(); //reset vertically
                    delay(5);                                                                   //Short delay                      
                    motorStep(coordinate1, pul3, dir3, 0, lateralSpeed);                        //Rotate lateral motor anticlockwise. !! Reminder to amend the steps and direction. Must be OPPOSITE to movement towards the magazine!
                    delay(5);                                                                   //Short delay before activating vertical system
                    motorStep(coordinate2, pul4, dir4, 1, verticalSpeed);                       //Rotate vertical motor anticlockwise. !!Remember to amend the steps and direction. Direction needs to be validated
                    delay(5);                                                                   //short delay
                    motorStep(coordinate3, pul2, dir2, 1, longitudinalSpeed);                   //Rotate longitudinal motor clockwise. !!Reminder to amend the steps and direction
                    delay(1000);                                                                //1s delay before closing gripper mouth
                    gripper(gripperOpenOutput);                                                 //Open Gripper Mouth
                    delay(1000);                                                                //1s delay before moving back to pickup area
                    //Serial.println("I am at the output magazine slot"); // For debugging
                    motorStep(outputLongitudinal,pul2,dir2,0,longitudinalSpeed);
                    delay(5);
                    gripper(gripperOutputClose);
                    //Bring gripper to input magazine
                    longitudinalReset();
                    delay(5);                                                                                      //short delay
                    verticalDeduction = glassSlideCount * glassSlideStepInput;
                    verticalReset();
                    delay(5);                                                                                      //short delay
                    motorStep(coordinate1 + lateralScan, pul3, dir3, 1, lateralSpeed);                             //Move laterally
                    delay(5);
                    motorStep(verticalPickup-verticalDeduction,pul4, dir4, 1, verticalSpeed);
                    gripper(forceClose);
                    delay(1000);                                                                                      //Short delay
                    //Serial.println("I am at the end of the loop for one slide"); //debug
                  }
                  if (magazine_num == 2 && magazineCount < 2) {
                    delay(1000);  //1 second delay before activating resets
                    longitudinalReset(); //Reset longitudinal arm to its limit switch position
                    verticalReset();// Reset vertical arm to its limit switch position
                    lateralReset(); //Reset lateral arm to its limit switch position
                    motorStep(lateralPickup+lateralScan, pul3, dir3, 0, lateralSpeed);  // Move lateral system to avoid hitting carrossel
                    carro(carroInput);  // rotate carrosel back to input position
                    Serial.print("Magazine");      // message sent to Raspi to indicate the end of the first magazine. Now awaiting "Inserted message"
                  } else {
                    //Serial.println("I am breaking out of inserted message while loop"); /For debugging
                    break;  // break out of the while loop for the inserted message
                  }
                }
              }
            }
            //Serial.println("I am breaking out of magazine_num while loop"); //For debugging
            break;  //break out of the while loop for magazine_num
          }
        }
      }
    }
    Serial.println("Finished");  //send message sent to ardRaspi to indicate a completed ,magazine
  }
}
