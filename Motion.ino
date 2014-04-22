

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
        temp = int(FrontSonar[midpoint]/58);
        if (temp < 20) return 999;
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
}

void sweep_gait()       // Non-blocking servo sweep for walking Gait...
{
  if(millis() > ServoControl) {
     if(servopos >=100) servodir = 0; 
     if(servodir == 1 && servopos < 130) servopos +=2;
     if(servopos <=40) servodir = 1; 
     if(servodir == 0 && servopos > 10) servopos -=2;
  
     gaitservo.write(servopos);              // tell servo to go to position in variable 'pos' 
     ServoControl = millis()+50;
  }  
}

