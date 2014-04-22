

static int m2c = 0;//counter for machine2

/****************************************************************************
State machine m1 is for motion control
the states are as follows:
    1  = Stopped
    2 = Forward
    3 = Avoiding Obstacle
    4 = Turn Left
    5 = Turn Right
    6 = Reverse
    
    MotionStop dictates how much time is spend in any state.
*/

State m1s1h(){                           // m1s1 is --- Motion:Stopped
  Serial.println("Motion:Stopped (State 1)");
  halt();
}//m1s1h()

State m1s1b(){
  if(m1.Timeout(5000)){                  // Maximum 5 seconds idle time
    m1.Set(m1s2h, m1s2b);
    Serial.println("changing to Motion:Forward (State 2)");
  };
}//m1s1b()

State m1s2h(){                          // m1s1 is ---  Moving Forward
  Serial.println("Motion: Forward (State 2)");
}//m1s2h()

State m1s2b(){                          
  if(m1.Timeout(MotionStop)){                  // Maximum 5 seconds forward motion
    m1.Set(m1s1h, m1s1b);
    Serial.println("changing back to Motion:Stopped (State 1)");
  }
  
  if(Distance > MINDIST){
   BlinkM_fadeToRGB( blinkm_addr, 0,255,0 );   // Display GREEN Status 
   forward();
  } else {
    Serial.println("changing to Motion:Avoid Obstacle (State 3)");
    m1.Set(m1s3h, m1s3b);
  }
}//m1s2b()

State m1s3h(){                           // m1s1 is ---  Avoid Obstacle
  Serial.println("Motion: Avoid Obstacle (State 3)");
  MotionStop = 200;                           // Set turn time for Obstacle Avoidance
  BlinkM_fadeToRGB( blinkm_addr, 255,0,0 );   // Display Warning RED 

}//m1s3h()

State m1s3b(){                          
  if(DistLeft > DistRight){
    m1.Set(m1s4h, m1s4b);            // Change State to Left Turn
  } else   if(DistRight > DistLeft){
    m1.Set(m1s5h, m1s5b);            // Change State to Right Turn
  } else {
    Serial.println("changing back to Motion:Stopped (State 1)");
    m1.Set(m1s1h, m1s1b);
  }  
}//m1s3b()


State m1s4h(){                          // m1s1 is ---  Turning Left
  Serial.println("Motion: Left Turn (State 4)");
}//m1s4h()

State m1s4b(){                          
  if(m1.Timeout(MotionStop)){                  // Maximum 5 seconds forward motion
    m1.Set(m1s1h, m1s1b);
    Serial.println("changing back to Motion:Stopped (State 1)");
  }
   BlinkM_fadeToRGB( blinkm_addr, 0,255,0 );   // Display GREEN Status 
   left();
}//m1s4b()


State m1s5h(){                          // m1s1 is ---  Turning Right
  Serial.println("Motion: Right Turn (State 5)");
}//m1s5h()

State m1s5b(){                          
  if(m1.Timeout(MotionStop)){                  // Maximum 5 seconds forward motion
    m1.Set(m1s1h, m1s1b);
    Serial.println("changing back to Motion:Stopped (State 1)");
  }
   BlinkM_fadeToRGB( blinkm_addr, 0,255,0 );   // Display GREEN Status 
   right();
}//m1s5b()


State m1s6h(){                          // m1s1 is --- Reverse
  Serial.println("Motion: Reversing (State 6)");
}//m1s6h()

State m1s6b(){                          
  if(m1.Timeout(MotionStop)){                  // Maximum 5 seconds forward motion
    m1.Set(m1s1h, m1s1b);
    Serial.println("changing back to Motion:Stopped (State 1)");
  }
   BlinkM_fadeToRGB( blinkm_addr, 0,255,0 );   // Display GREEN Status 
   reverse();
}//m1s6b()

/*****************************************************************************/


State m2s1h(){
    Serial.print("Machine2:State1:Count");
    Serial.println(m2c++);
}//m2s1h()

State m2s1b(){
  if(m2.Timeout(200)) m2.Set(m2s1h, m2s1b);
  if(m2c>10) m2.Finish();
}//m2s1b()

State m3s1h(){
  Serial.println("Machine3:state1");
}//m3s1h()

State m3s1b(){
  if(m3.Timeout(250)) m3.Set(m3s2h, m3s2b);
}//m3s1b()

State m3s2h(){
  Serial.println("Machine3:state2");
}//m2s2h()

State m3s2b(){
  if(m3.Timeout(250)) m3.Set(m3s3h, m3s3b);
}//m3s2b()

State m3s3h(){
  Serial.println("Machine3:state3");
}

State m3s3b(){
  if(m3.Timeout(250)) m3.Finish();
}//s3m1()


/*****************************************************************************/

State On(){
    RightLedState = !RightLedState;   // Toggle Red/Blue LED state
    LeftLedState = !LeftLedState;
    digitalWrite(rightled,RightLedState);
    digitalWrite(leftled,LeftLedState);
  if(Leds.Timeout(2000)) Leds.Set(Off);
}

State Off(){
    RightLedState = !RightLedState;   // Toggle Red/Blue LED state
    LeftLedState = !LeftLedState;
    digitalWrite(rightled,RightLedState);
    digitalWrite(leftled,LeftLedState);
  if(Leds.Timeout(2000)) Leds.Set(On);
}
