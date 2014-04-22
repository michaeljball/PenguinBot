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

#define MINDIST   30        // Distance threshold for object Avoidance   
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

