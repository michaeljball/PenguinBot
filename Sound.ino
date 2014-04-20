

void listen() {                          // Listen to the Microphone for a threshold value
  long sumOfSquares = 0; 
  for (int i=0; i<numberOfSamples; i++) { //take many readings and average them
    sample = analogRead(Microphone);               //take a reading
    signal = (sample - middleValue);      //work out its offset from the center
    signal *= signal;                     //square it to make all values positive
    sumOfSquares += signal;               //add to the total
  }
  averageReading = sumOfSquares/numberOfSamples;     //calculate running average
  runningAverage=(((averagedOver-1)*runningAverage)+averageReading)/averagedOver;

if (runningAverage>threshold){         //is average more than the threshold ?
    SoundFlag = 1;
  }else{
    SoundFlag = 0;
  }
//  Serial.print(sample);Serial.print("   "); Serial.print(threshold);   Serial.print("   "); Serial.println(runningAverage);        //print the value so you can check it
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




