 /*
 PenguinBot_1    Hacking a Noisy Stuffed Penguin Toy.
 UNIX_GURU at hotmail dot com
 or @unix_guru on Twitter
 http://arduino-pi.blogspot.ca/2014/04/penguinbot-fun-weekend-arduino-project.html

 https://github.com/michaeljball/PenguinBot

 Proportional Steering
 Servomotor to rock body back and forth.
 Two photocells to turn toward and follow light.   ***** Not yet Implemented
 Sound Activation... for... something... ?         ***** Not yet Implemented
 Crazy penguin sounds and spooky music.
 Sonar Obstacle avoidance.
 
 Most functions/Operations are non-blocking...
 
 Given enough time, I would have employed Metro Scheduler http://playground.arduino.cc/Code/Metro
 and a true Finite State Machine like  http://playground.arduino.cc/Code/FiniteStateMachine
 
 */
 
#define Sonar       6             // Maxbotix EZ-1 
#define Microphone  A1            // Condeser mic with preamp
#define RightPhoto  A2            // Right Photocell 100k
#define LeftPhoto   A3            // Left Photocell 100k
#define SDAPIN      A4            // I2C SDA
#define SCLPIN      A5            // I2C SCL

#define servopin  9

#define soundpin  5          // Enable Penguin sounds out
#define rightled  7          // Blue/Red led in right eye
#define leftled   8         // Blue/Red led in left eye

#define leftspd   11          // Left Motor Speed
#define leftdir   13         // Left Motor Direction
#define rightspd   3          // Right Motor Speed
#define rightdir  12         // Right Motor Direction

#define MINDIST   200
#define IDLEMAX   10000
#define TURNTIME  1000       // Time it takes in millis to turn 90 degrees - trial and error
 
//#include <Metro.h> // Include Metro library
#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
                 
int servopos = 90;    // variable to store the servo position 
int servodir = 1;     // Incrementing clockwise..

int Spd = 100;              // Motor speed from 0 - 255

unsigned int LeftEye;      // Left Photocell Value
unsigned int RightEye;     // Right Photocell Value
unsigned int Distance;     // Distance to nearest obstacle directly in front
unsigned int DistRight;     // Distance to nearest obstacle to the right
unsigned int DistLeft;     // Distance to nearest obstacle to the left

unsigned int Volume;       // Volume from microphone 


// Create variables to hold the various states
int RightLedState = HIGH;
int LeftLedState = LOW;
int MotionState = 0;          // Keep state of current motion.
int ScanState = 0;            // State Counter required to make look_left_right() non blocking


unsigned long LedControl; 
unsigned long ServoControl; 
unsigned long MotorControl; 
unsigned long MotionStop;     // Parameter used to halt motion at a predetermined time.
unsigned long SoundStop;      // Parameter used to halt sound at a predetermined time.
unsigned long IdleTime;       // Parameter used to restart motion at a predetermined time.
unsigned long ScanTime;       // Parameter used to tell bot when to scan left/right.
unsigned long ScanDelay;      // Parameter used to allow bot to reach proper angle when scanning left/right.


// ******************************************************************************************
void setup() {   

  Serial.begin (115200);
  Serial.println("RST, Running PenguinBot_02 140413");
  
  // initialize the pins.
  pinMode(soundpin, OUTPUT);     
  pinMode(rightled, OUTPUT);     
  pinMode(leftled, OUTPUT);  

  pinMode(leftspd, OUTPUT);  
  pinMode(leftdir, OUTPUT);  
  pinMode(rightspd, OUTPUT);  
  pinMode(rightdir, OUTPUT);  

  pinMode(RightPhoto, INPUT);  
  pinMode(LeftPhoto, INPUT);  


  pinMode(Sonar, INPUT);                    // Pod Sonar MAXSonar 
  digitalWrite(Sonar, LOW);
 
  Serial.println("MaxSonar Sensors attached");

  pinMode(Microphone, INPUT);                    // Sound activation
  digitalWrite(Microphone, HIGH);

  myservo.attach(servopin);  // attaches the servo on pin 9 to the servo object 
  
  LedControl = millis()+1500;
  ServoControl = millis()+20;
  MotorControl = millis()+2000;
  MotionStop = millis()+2000;      // Arbitrarily set this to 2 seconds of motion
  IdleTime = millis()+IDLEMAX;      // Arbitrarily set this to 15 seconds of Idle
  ScanTime = millis()+5000;      // Time between each scan.
  ScanDelay = millis()+1000;      // Time to allow Bot to reach goal.
  
}

// the loop routine runs over and over again forever:
void loop() {
  
  get_sensors();
  motion_control();
  provide_feedback();
  sweep_servo();
 // chirp();
  
}


void motion_control()
{
  if(MotionState > 0 && millis() > MotionStop) {  // If we have exceeded the requested motion time, then halt.
     halt();
     MotionState = 0;
     IdleTime = millis()+IDLEMAX;      // Arbitrarily set this to 15 seconds of Idle
  }
  
  if(millis() > ScanTime) {            // Every 5 seconds stop and look left/right
    look_right_left();
  }  
  
  switch(MotionState)
  {
    case 0:                               // Stopped. 
      if(Distance < MINDIST){             // If we are already stopped, but something is coming at us
        MotionStop = millis()+1000;        // Back up briefly...
        MotionState = 2;
        reverse();
      } else if(millis() > IdleTime) {    // If idle longer than IDLEMAX (10 seconds)
        MotionStop = millis()+2000;      // Move forward 2 seconds...
        MotionState = 1;
        forward();
      }  
      break;
    case 1:                               // Moving forward looking for light, avoiding objects
      if(Distance < MINDIST){             // If there is an obstacle in the way
        halt();
        if(DistLeft > DistRight && DistLeft > MINDIST){   // If more room to the Left, go that way
          MotionStop = millis()+TURNTIME ;        
          MotionState = 2;
          left();
        } else if (DistRight > DistLeft && DistRight > MINDIST){    // If more room to the right, go that way
          MotionStop = millis()+TURNTIME ;        
          MotionState = 2;
          right();
        }  else {                                          // Otherwise turn right around.
          MotionStop = millis()+(TURNTIME*2);        
          MotionState = 2;
          right();
       } 
      }        
      break;
    case 2:                               // Avoiding obstacle, mid turn 
      if(millis() > MotionStop) {    // If we have completed our turn
        MotionStop = millis()+2000;      // Move forward 2 seconds...
        MotionState = 1;
        forward();
      }  
      break;
      
  }
}


void reverse()
{
    digitalWrite(leftdir,HIGH);    analogWrite(leftspd,Spd);
    digitalWrite(rightdir,HIGH);   analogWrite(rightspd,Spd);
}


void forward()
{
    digitalWrite(leftdir,LOW);    analogWrite(leftspd,Spd);
    digitalWrite(rightdir,LOW);   analogWrite(rightspd,Spd);
}

void right()
{
    digitalWrite(leftdir,HIGH);    analogWrite(leftspd,Spd);
    digitalWrite(rightdir,LOW);    analogWrite(rightspd,Spd);
}
void left()
{
    digitalWrite(leftdir,LOW);     analogWrite(leftspd,Spd);
    digitalWrite(rightdir,HIGH);   analogWrite(rightspd,Spd);
}

void halt()
{
    digitalWrite(leftdir,LOW);     analogWrite(leftspd,0);
    digitalWrite(rightdir,LOW);    analogWrite(rightspd,0);
}

void get_sensors()
{
  LeftEye = analogRead(LeftPhoto)*67;            // Temporary compensation for mismatched photocells
  RightEye = analogRead(RightPhoto)*100;
  Distance = pulseIn(Sonar, HIGH)/5.8;
  Volume = analogRead(Microphone);
   
}
  
void look_right_left()                          // This is a non-blocking process that iterates through a series
{                                               // of states to get left, forward, right distances periodically
  if(millis() > ScanDelay) {
      switch(ScanState)
      {
        case 0:
           Distance = pulseIn(Sonar, HIGH)/5.8;      // Get distance directly in front.
           right();
           ScanState++;
           break;
        case 1:
           halt();
           DistRight = pulseIn(Sonar, HIGH)/5.8;      // Get distance to the right.
           left();
           ScanState++;
           break;
        case 2:
           halt();
           Distance = pulseIn(Sonar, HIGH)/5.8;      // Get distance directly in front again (just to make states easier)
           left();
           ScanState++;
           break;
        case 3:
           halt();
           DistLeft = pulseIn(Sonar, HIGH)/5.8;      // Get distance to the left.
           right();                                  // Straighten ourselves up again (hopefully... no encoders, so...)
           ScanState++;
           break;
        case 4:
           halt(); 
           ScanState = 0;                           // Reset for the next run. 
           ScanTime = millis()+5000;
           break;
      }
  }      
}

void provide_feedback()
{
  if(SoundStop > 0 && millis() > SoundStop) {
    digitalWrite(soundpin, LOW);
    SoundStop = 0;
  }  
//  digitalWrite(sound, HIGH);   // turn the LED on (HIGH is the voltage level)
  if (millis() > LedControl) { // check if the metro has passed its interval .
    RightLedState = !RightLedState;   // Toggle LED state
    LeftLedState = !LeftLedState;
    digitalWrite(rightled,RightLedState);
    digitalWrite(leftled,LeftLedState);
    LedControl = millis()+1000;    // Reset the timer.

    Serial.print("Left Eye: ");  Serial.print(LeftEye); Serial.print("  Right Eye: ");  Serial.print(RightEye);  
    Serial.print("   Distance: ");  Serial.print(Distance); Serial.print("mm    ");   
    Serial.print("Volume: ");  Serial.print(Volume);  
    Serial.print("   State: ");  Serial.print(MotionState); Serial.println("  "); 
  }
  
}

void sweep_servo()       // Non-blocking servo sweep.
{
  if(millis() > ServoControl) {
     if(servopos >=130) servodir = 0; 
     if(servodir == 1 && servopos < 130) servopos +=5;
     if(servopos <=40) servodir = 1; 
     if(servodir == 0 && servopos > 40) servopos -=5;
  
     myservo.write(servopos);              // tell servo to go to position in variable 'pos' 
     ServoControl = millis()+50;
  }  
}

void make_noise(int period)              // I have scavenged the original sound module from the toy. 
{                                        // it only takes 20ma, so I'm powering it directly from an Arduino Pin
    digitalWrite(soundpin, HIGH);
    SoundStop = millis()+period;
}

void chirp()                            // The first 2 seconds of sound are a cute  chirp
{
  make_noise(2000);
}

void bark()                           // The first 5 seconds of sound are a cute  chirp and a "bark like" sound
{
  make_noise(5000);
}

void annoy()                           // After The first 5 seconds of sound... you will loose your mind...
{
  make_noise(12000);
}

