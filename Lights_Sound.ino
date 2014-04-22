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

  if (millis() > ReportTime) {        // check if the metro has passed its interval .
    ReportTime = millis()+1000;    // Reset the timer.

    Serial.print("Left Eye: ");  Serial.print(LeftEye); Serial.print("  Right Eye: ");  Serial.print(RightEye);  
    Serial.print("   Distance: ");  Serial.print(Distance); Serial.print("cm    ");   
    Serial.print("Left: ");  Serial.print(DistLeft); Serial.print("cm    ");   
    Serial.print("Right: ");  Serial.print(DistRight); Serial.print("cm    ");   
    Serial.print("Volume: ");  Serial.print(sample);      Serial.print(" DC Level: ");  Serial.print(signal); 
    Serial.print("  RAVG: ");  Serial.print(runningAverage);      Serial.print("  Thresh: ");  Serial.println(threshold); 
  }
  
}


void display_BlinkM()
{
  // Control the BlinkM RGB led from environmental characteristics
  // for the "Personality" trait, map Distance to nearest object as RED
  // map the average of Left/Right ambient Light for BLUE
  // And map the ambient sound to GREEN
  byte r = map(Distance, 2400, 400, 255, 100);
  byte g = map(sample, 300, 1023, 128, 255);               
  byte b = map((LeftEye+RightEye)/2, 0, 700, 255, 0);
 // BlinkM_setRGB( blinkm_addr, r,g,b );
  BlinkM_fadeToRGB( blinkm_addr, r,g,b );

}


void listen() {                          // Listen to the Microphone for a threshold value
  long sumOfSquares = 0; 
  for (int i=0; i<numberOfSamples; i++) { //take many readings and average them
    sample = analogRead(Microphone);               //take a reading
    signal = (sample - middleValue);      //work out its offset from the center
    signal = abs(signal);                     //square it to make all values positive
    sumOfSquares += signal;               //add to the total
  }
  averageReading = sumOfSquares/numberOfSamples;     //calculate running average
  runningAverage=(((averagedOver-1)*runningAverage)+averageReading)/averagedOver;

//    Serial.print("Volume: ");  Serial.print(sample);      Serial.print(" DC Level: ");  Serial.print(signal); 
//    Serial.print("  RAVG: ");  Serial.print(runningAverage);      Serial.print("  Thresh: ");  Serial.println(threshold); 
if (runningAverage<threshold){         //is average more than the threshold ?
    SoundFlag = 1;
    Serial.print(".");
  }else{
    SoundFlag = 0;
  }
//  Serial.print(sample);Serial.print("   "); Serial.print(threshold);   Serial.print("   "); Serial.println(runningAverage);        //print the value so you can check it
}

void listen2()
{
unsigned long startMillis= millis(); // Start of sample window
unsigned int peakToPeak = 0; // peak-to-peak level
 
unsigned int signalMax = 0;
unsigned int signalMin = 1024;
     
    while (millis() - startMillis < 50)
    {
      sample = analogRead(Microphone);
      if (sample < 1024) // toss out spurious readings
      {
      if (sample > signalMax)
      {
      signalMax = sample; // save just the max levels
      }
      else if (sample < signalMin)
      {
      signalMin = sample; // save just the min levels
      }
      }
    }
    peakToPeak = signalMax - signalMin;
    Serial.print("Peak to Peak: ");    Serial.println(sample);


}

void make_noise(int period)              // I have scavenged the original sound module from the toy. 
{                                        // it only takes 20ma, so I'm powering it directly from an Arduino Pin
    digitalWrite(soundpin, HIGH);
    SoundStop = millis()+period;
}

void random_noise()
{
  switch(random(1, 4)) {
     case 1:
      tweet();
      break;
     case 2:
      chirp();
      break;
     case 3:
      bark();
      break;
     case 4:
     // annoy();
      break;
  }  
}

void tweet()                            // The first 2 seconds of sound are a cute  chirp
{
  make_noise(650);
}

void chirp()                            // The first 2 seconds of sound are a cute  chirp
{
  make_noise(1500);
}

void bark()                           // The first 5 seconds of sound are a cute  chirp and a "bark like" sound
{
  make_noise(4000);
}

void annoy()                           // After The first 5 seconds of sound... you will loose your mind...
{
  make_noise(12000);
}




