

void motion_control()
{
  if(MotionState > STOPPED && millis() > MotionStop) {  // If we have exceeded the requested motion time, then halt.
     Serial.print("Halting: ");  Serial.print(Distance); Serial.println("mm    ");  
     halt();
     MotionState = STOPPED;
     IdleTime = millis()+IDLEMAX;      // Arbitrarily set this to 15 seconds of Idle
  }
    
  switch(MotionState)
  {
    case STOPPED:                         // Stopped. 
/*
      if(Distance < MINDIST){             // If we are already stopped, but something is coming at us
        MotionState = APPROACHED;
        MotionStop = millis()+100;        // Back up briefly...
        reverse();
        break;
      } else */
      
      if(GameState) {
        if(RightPhoto > LeftPhoto-5 && RightPhoto < LeftPhoto+5 ) { right();    // give a window of 10
        } else if(LeftPhoto > RightPhoto-5  && LeftPhoto < RightPhoto+5 ) { left();
        } else halt();
        MotionStop = millis()+100;        // Back up briefly...        
      }  
      if(millis() > IdleTime && Distance > MINDIST  && ManualState) {    // If idle longer than IDLEMAX (10 seconds)
        MotionState = FWDCLR;            // AND we have distance in front, transition to Forward Clear state        
        MotionStop = millis()+20000;      // If motionStop not defined set to 2 seconds...
        forward();
      }  
      break;
      
    case APPROACHED:                    // Stopped, but object approaching too close 
      break;
      
    case FWDCLR :                                             // Assessing ability to move
      if(Distance < MINDIST && millis() < MotionStop){        // If there is an obstacle in the way
        halt();
         Serial.print("Blocked: ");  Serial.print(Distance); Serial.println("mm    ");
        // Assess best direction to turn. 
        if(DistLeft > DistRight && DistLeft > MINDIST){       // More room to the Left, Turn Left
          MotionStop = millis()+600;        
          MotionState = FWDBLKTL;
          left();
        } else if (DistRight > DistLeft && DistRight > MINDIST){   // More room to the Right, Turn Right
        }  else {                                            // Otherwise turn around
          MotionStop = millis()+600;        
          MotionState = FWDBLKTA;
          right();
       } 
      } else {
         if(MotionStop < millis()) MotionStop = millis()+20000;      // If motionStop not defined set to 2 seconds...
         forward();
     }    
      break;
      
    case FWDBLKTL:                        // Forward is blocked, turning left
      left();
      break;
      
    case FWDBLKTR:                        // Forward is blocked, turning right
      right();
      break;
      
   case FWDBLKTA:
      left();
      break;
      
    case REVERSE:
      if(MotionStop < millis()) MotionStop = millis()+2000;      // If motionStop not defined set to 2 seconds...
      reverse();
      break;
  }
}


void forward()
{
    digitalWrite(leftdir,HIGH);    analogWrite(leftspd,Spd);
    digitalWrite(rightdir,HIGH);   analogWrite(rightspd,Spd-20);
}


void reverse()
{
    digitalWrite(leftdir,LOW);    analogWrite(leftspd,Spd);
    digitalWrite(rightdir,LOW);   analogWrite(rightspd,Spd);
}

void left()
{
    digitalWrite(leftdir,HIGH);    analogWrite(leftspd,Spd);
    digitalWrite(rightdir,LOW);    analogWrite(rightspd,Spd);
}
void right()
{
    digitalWrite(leftdir,LOW);     analogWrite(leftspd,Spd);
    digitalWrite(rightdir,HIGH);   analogWrite(rightspd,Spd);
}

void halt()
{
    digitalWrite(leftdir,LOW);     analogWrite(leftspd,0);
    digitalWrite(rightdir,LOW);    analogWrite(rightspd,0);
}


void look_left_right()                     // This is a non-blocking process that iterates through a series
{                                          // of states to get left, forward, right distances periodically
  switch(ScanState)
  {
    case 0:                            // Get distance directly in front.
       Distance = get_distance();      
       sonarservo.write(120);
       ScanState++;
       break;
    case 1:                            // Get distance to the right.
       DistRight = get_distance();
       sonarservo.write(70);
       ScanState++;
       break;
    case 2:                            // Get distance directly in front again (just to make states easier)
       Distance = get_distance();
       sonarservo.write(0);
       ScanState++;
       break;
    case 3:                            // Get distance to the left.
       DistLeft = get_distance();
       sonarservo.write(70);           // Set Sonar to straight ahead again.
       ScanState = 0;                  // Reset for the next run. 
       break;
  }
}


int get_distance()
{

  int temp; 
        for(int i = 0; i < arraysize; i++)
        {                                                    //array pointers go from 0 to 4
          FrontSonar[i] = pulseIn(Sonar, HIGH);
          delay(7);  //wait between pulses
        }        
        isort(FrontSonar, arraysize);   
        temp = int(FrontSonar[midpoint]/5.8);
        if (temp < 300) return 999;
        return temp;

 //  return int (pulseIn(Sonar, HIGH)/5.8);  
}



//*********************************************************************************
// sort function
void isort( int *a, int n)
               //  *a is an array pointer function
{
  for (int i = 1; i < n; ++i)
  {
    int j = a[i];
    int k;
    for (k = i - 1; (k >= 0) && (j < a[k]); k--)
    {
      a[k + 1] = a[k];
    }
    a[k + 1] = j;
  }
}

void follow_light()
{

  if(MotionState == STOPPED)
        if(RightEye > LeftEye-10 && RightEye < LeftEye+10 ) { right();    // give a window of 10
        } else if(LeftEye > RightEye-10  && LeftEye < RightEye+10 ) { left();
        } else halt();
        MotionStop = millis()+100;        // Back up briefly...        
        } 
  } 
}


