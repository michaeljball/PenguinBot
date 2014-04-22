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
 

#include "PenguinBot.h"
#include "Wire.h"
#include "BlinkM_funcs.h"

#define blinkm_addr 0x00

#include <SM.h>
SM m1(m1s1h, m1s1b);//machine1
SM m2(m2s1h, m2s1b);//machine2
SM m3(m3s1h, m3s1b);//machine3
SM Leds(On);//machine to blink led continously


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

long int threshold=400;          //at what level the light turns on
boolean SoundFlag =0;              // Is Sound level greater than Threshold.

// Create variables to hold the various states
int RightLedState = HIGH;
int LeftLedState = LOW;
//int MotionState = 0;          // Keep state of current motion.
//int prevMotionState = 0;          // Keep state of current motion.
int ScanState = 0;            // State Counter required to make look_left_right() non blocking
int VoiceState = 0;            // Voice state boolean quiet or annoying
int GameState = 0;            // Game state boolean turn towards light or not
int ManualState = 0;            // Game state boolean turn towards light or not


unsigned long ReportTime; 
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
  Serial.println("RST, Running PenguinBot_05 140421");
  
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
  sbi(ADCSRA,ADPS2) ;   cbi(ADCSRA,ADPS1) ;   cbi(ADCSRA,ADPS0) ;

  pinMode(Microphone, INPUT);                    // Sound activation
  // digitalWrite(Microphone, HIGH);
  
  ReportTime = millis()+1000;
  MotionStop = 2000;                // Arbitrarily set this to 2 seconds of motion
  ScanDelay = millis()+300;        // Time to allow Bot to reach goal.
  SoundDelay = millis()+5000;      // Time to allow Bot to reach goal.
  
  randomSeed(analogRead(0));
  //listen(1);                      // Initialize ambient sound threshold
  
  // Start communications with BlinkM RGB LED module
  BlinkM_begin();
 // BlinkM_stopScript( blinkm_addr );  // turn off factory startup script
  BlinkM_doFactoryReset();;
 // BlinkM_setFadeSpeed(blinkm_addr, 128);  // Increase Fade speed.

  
}

// the loop routine runs over and over again forever:
void loop() {
  
  get_sensors();
  EXEC(m1);
  EXEC(Leds);//blink leds concurrently for ever

  provide_feedback();
  if (SonarMetro.check()) look_left_right();           // Sonar scan left/right
  if (GaitMetro.check())  sweep_gait();                // Rocking motion of body
  if (GameMetro.check() && GameState)  follow_light(); // If in Game mode, turn to light

 
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
  // listen();
  
  LeftEye = analogRead(LeftPhoto);           
  RightEye = analogRead(RightPhoto);
  if(ScanState == 0 || ScanState == 2) {   // If we are currently looking ahead.
      Distance = get_distance();
      display_BlinkM();
  }
}
  


