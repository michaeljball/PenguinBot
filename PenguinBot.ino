 /*
 PenguinBot_1    Hacking a Noisy Stuffed Penguin Toy.
 UNIX_GURU at hotmail dot com
 or @unix_guru on Twitter
 http://arduino-pi.blogspot.ca/2014/04/penguinbot-fun-weekend-arduino-project.html
 
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


#define soundpin  4          // Enable Penguin sounds out
#define sonarsrv  5          // Sonar servo to scan left / right..
#define rightled  7          // Blue/Red led in right eye
#define leftled   8          // Blue/Red led in left eye
#define headsrv  9          // Head bobbing Servo

#define leftspd   11          // Left Motor Speed
#define leftdir   13         // Left Motor Direction
#define rightspd   3          // Right Motor Speed
#define rightdir  12         // Right Motor Direction

#define MINDIST   400        // Distance threshold for object Avoidance   
#define IDLEMAX   6000      // Maximum time allowed for idle
#define BufferLength 32      // Serial buffer for commands


// Define Motion States here
#define STOPPED     0        // No motion, waiting for idle timer to run out
#define APPROACHED  1        // Obstacle approaching - back up
#define FWDCLR      2        // Moving Forward, distance is clear
#define FWDBLK      3        // Moving Forward, blocked, move to avoidance
#define FWDBLKTL    4        // Moving Forward, blocked, turn left
#define FWDBLKTR    5        // Moving Forward, blocked, turn right
#define FWDBLKTA    6        // Moving Forward, blocked, turn around
#define REVERSE     7        // Moving Backwards

 
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/* 
* BlinkM connections to Arduino
* PWR - -- gnd -- black -- Gnd
* PWR + -- +5V -- red   -- 5V
* I2C d -- SDA -- green -- Analog In 4
* I2C c -- SCK -- blue  -- Analog In 5
*/

 
#include "Wire.h"
#include "BlinkM_funcs.h"

#define blinkm_addr 0x00


#include <Metro.h> // Include Metro library
#include <Servo.h> 
 
Servo gaitservo;              // create servo object to control walking gait 
Servo sonarservo;             // create servo object to control sonar sweep               
int servopos = 90;            // variable to store the gait servo position 
int servodir = 1;             // Incrementing clockwise..

int Spd = 150;              // Motor speed from 0 - 255

int LeftEye;      // Left Photocell Value
int RightEye;     // Right Photocell Value

// ****************************** Ranging Variables *********************************
int FrontSonar[] = {0, 0, 0, 0, 0};      // Distance read by front Sonar
int arraysize = 5;  //quantity of values to find the median (sample size). Needs to be an odd number
int midpoint;
int Distance;     // Distance to nearest obstacle directly in front
int DistRight;     // Distance to nearest obstacle to the right
int DistLeft;     // Distance to nearest obstacle to the left

// From Arduino Cookbook - Detecting Sound
const int middleValue = 512;      //the middle of the range of analog values
const int numberOfSamples = 64;  //how many readings will be taken each time

int sample;                       //the value read from microphone each time
long signal;                      //the reading once you have removed DC offset
long averageReading;              //the average of that loop of readings

long runningAverage=0;            //the running average of calculated values
const int averagedOver= 8;       //how quickly new values affect running average

long int threshold=50000;          //at what level the light turns on
boolean SoundFlag =0;              // Is Sound level greater than Threshold.

// Create variables to hold the various states
int RightLedState = HIGH;
int LeftLedState = LOW;
int MotionState = 0;          // Keep state of current motion.
int prevMotionState = 0;          // Keep state of current motion.
int ScanState = 0;            // State Counter required to make look_left_right() non blocking
int VoiceState = 0;            // Voice state boolean quiet or annoying
int GameState = 0;            // Game state boolean turn towards light or not
int ManualState = 0;            // Game state boolean turn towards light or not


unsigned long LedControl; 
unsigned long ServoControl; 
unsigned long MotorControl; 
unsigned long MotionStop;     // Parameter used to halt motion at a predetermined time.
unsigned long SoundStop;      // Parameter used to halt sound at a predetermined time.
unsigned long IdleTime;       // Parameter used to restart motion at a predetermined time.
unsigned long ScanTime;       // Parameter used to tell bot when to scan left/right.
unsigned long ScanDelay;      // Parameter used to allow bot to reach proper angle when scanning left/right.
unsigned long SoundDelay;      // Parameter used to allow bot to reach proper angle when scanning left/right.

Metro SonarMetro = Metro(1000);
Metro GaitMetro = Metro(20);
Metro GameMetro = Metro(1000);

// ================================== Variables related to Command Processing ==============================================================

        
String Command = "";
String Parameter = "";
char inData[BufferLength];                                 // Buffer for the incoming data
char *inParse[BufferLength];                               // Buffer for the parsed data chunks

String inString = "";                                      // Storage for data as string
int index = 0;
boolean stringComplete = false;




// ******************************************************************************************
void setup() {   

  Serial.begin (115200);
  Serial.println("RST, Running PenguinBot_03 140417");
  
  // initialize the pins.
  pinMode(soundpin, OUTPUT);     
  pinMode(rightled, OUTPUT);   pinMode(leftled, OUTPUT);  

  pinMode(leftspd, OUTPUT);    pinMode(leftdir, OUTPUT);  
  pinMode(rightspd, OUTPUT);   pinMode(rightdir, OUTPUT);  

  pinMode(RightPhoto, INPUT);  pinMode(LeftPhoto, INPUT);  

  pinMode(Sonar, INPUT);                    // Pod Sonar MAXSonar 
 
  Serial.println("MaxSonar Sensors attached");

  gaitservo.attach(headsrv);    // attaches the servo on pin 9 to the servo object 
  sonarservo.attach(sonarsrv);  // attaches the servo on pin 5 to the servo object 
  sonarservo.write(70);         // Should be 90... compensating for a assembly issue...
  Distance = get_distance();    // Get current distance value


  // set ADC prescale to 16 giving a 1MHZ clock or 77khz sample rate
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;

  pinMode(Microphone, INPUT);                    // Sound activation
  digitalWrite(Microphone, HIGH);
  
  
  LedControl = millis()+1500;
  MotionStop = millis()+2000;      // Arbitrarily set this to 2 seconds of motion
  IdleTime = millis()+IDLEMAX;      // Arbitrarily set this to 15 seconds of Idle
  ScanDelay = millis()+300;      // Time to allow Bot to reach goal.
  SoundDelay = millis()+5000;      // Time to allow Bot to reach goal.
  
  randomSeed(analogRead(0));
  //listen(1);                      // Initialize ambient sound threshold
  
  // Start communications with BlinkM RGB LED module
  BlinkM_begin();
  BlinkM_stopScript( blinkm_addr );  // turn off factory startup script
  BlinkM_setFadeSpeed(blinkm_addr, 128);  // Increase Fade speed.

  
}

// the loop routine runs over and over again forever:
void loop() {
  
  get_sensors();
  motion_control();
  provide_feedback();
  if (SonarMetro.check()) look_left_right();           // Sonar scan left/right
  if (GaitMetro.check())  sweep_gait();                // Rocking motion of body
  if (GameMetro.check() && GameState)  follow_light(); // If in Game mode, turn to light

//  sweep_gait();                // Strictly personality.. no real "function"
 
  SerialEvent();              // Grab characters from Serial
  
  // =======================   if serial data available, process it ====================
  if (stringComplete)                   // if there's any serial available, read it:
  {
    ParseSerialData();                  // Parse the recieved data
    inString = "";                      // Reset inString to empty   
    stringComplete = false;             // Reset the system for further input of data
  }  
 
}


void get_sensors()
{
  listen();
  
  LeftEye = analogRead(LeftPhoto);           
  RightEye = analogRead(RightPhoto);
  if(ScanState == 0 || ScanState == 2) {   // If we are currently looking ahead.
      Distance = get_distance();
      display_BlinkM();
  }
}
  

void provide_feedback()
{
  if(SoundStop > 0 && millis() > SoundStop) {       // Ensure sound ends when timer reached.
    digitalWrite(soundpin, LOW);
    SoundStop = 0;
  }  
  
  if(VoiceState > 0 && millis() > SoundDelay) {                      // Randomly chose a "Penguin" sound to play.
    random_noise();
    SoundDelay = millis()+7000;
  }  

  if (millis() > LedControl) {        // check if the metro has passed its interval .
    RightLedState = !RightLedState;   // Toggle Red/Blue LED state
    LeftLedState = !LeftLedState;
    digitalWrite(rightled,RightLedState);
    digitalWrite(leftled,LeftLedState);
    LedControl = millis()+1000;    // Reset the timer.

    Serial.print("Left Eye: ");  Serial.print(LeftEye); Serial.print("  Right Eye: ");  Serial.print(RightEye);  
    Serial.print("   Distance: ");  Serial.print(Distance); Serial.print("mm    ");   
    Serial.print("   Left: ");  Serial.print(DistLeft); Serial.print("mm    ");   
    Serial.print("   Right: ");  Serial.print(DistRight); Serial.print("mm    ");   
    Serial.print("Volume: ");  Serial.print(sample);  
    Serial.print("   State: ");  Serial.print(MotionState); Serial.println("  "); 
  }
  
}

void sweep_gait()       // Non-blocking servo sweep for walking Gait...
{
  if(millis() > ServoControl) {
     if(servopos >=100) servodir = 0; 
     if(servodir == 1 && servopos < 100) servopos +=2;
     if(servopos <=10) servodir = 1; 
     if(servodir == 0 && servopos > 10) servopos -=2;
  
     gaitservo.write(servopos);              // tell servo to go to position in variable 'pos' 
     ServoControl = millis()+50;
  }  
}

void display_BlinkM()
{
  // Control the BlinkM RGB led from environmental characteristics
  // for the "Personality" trait, map Distance to nearest object as RED
  // map the average of Left/Right ambient Light for BLUE
  // And map the ambient sound to GREEN
  if(MotionState == APPROACHED) {
      BlinkM_fadeToRGB( blinkm_addr, 255,0,0 );   // Display Warning RED 
  } else {
  
      byte r = map(Distance, 2400, 400, 255, 100);
      byte g = map(sample, 300, 1023, 128, 255);               
      byte b = map((LeftEye+RightEye)/2, 0, 700, 255, 0);
     // BlinkM_setRGB( blinkm_addr, r,g,b );
      BlinkM_fadeToRGB( blinkm_addr, r,g,b );
  }
}

