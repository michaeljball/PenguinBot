/*  
Available Commands:

f = forward
b = reverse
l = left
r = right
x = stop
s = Speed
v = toggle voice
g = light following game

*/

void ParseSerialData()
{

  char *p = inData;                // The data to be parsed
  char *str;                       // Temp store for each data chunk
  int count = 0;                   // Id ref for each chunk
    
  while ((str = strtok_r(p, ",", &p)) != NULL)    // Loop through the data and seperate it into chunks at each "," delimeter
  { 
    inParse[count] = str;      // Add chunk to array  
    count++;      
  }

  if(count == 2)     // If the data has two values then..  
  {
    Command = inParse[0];       // Define value 1 as a Command identifier
    Parameter = inParse[1];     // Define value 2 as a Parameter value

    processCommand();
  }
}
 
 

  
void processCommand()
{
char buf[32]; // make this at least big enough for the whole string
    Parameter.toCharArray(buf, sizeof(buf));        // Convert String to Character array  

    Serial.print("CMD,"); Serial.print(Command); Serial.print(","); Serial.println(Parameter);
    // Call the relevant identified Commandtion  
    switch(Command[0])
    {
      case 'f':                                                         // Move Forward "Parameter" ticks
        MotionStop = millis()+atoi(buf);
        MotionState = FWDCLR;
        forward();
      break;

       case 'b': 
        MotionStop = millis()+atoi(buf);
        MotionState = REVERSE;
        reverse();
       break;

       case 'r': 
        MotionStop = millis()+atoi(buf);
        MotionState = FWDBLKTR;
        right();
        break;

       case 'l': 
        MotionStop = millis()+atoi(buf);
        MotionState = FWDBLKTL;
        left();
        break;

      case 'v': 
        VoiceState = !VoiceState;      // Toggle Voice state, quiet or annoying
        break;

       case 'x': 
        MotionStop = 0;
        MotionState = STOPPED;
        halt();
        break;
        
       break;

       case 's':                                         // Set Desired Speed
         Spd = atoi(buf);
        break;

      case 'g': 
        GameState = !GameState;      // Toggle Voice state, quiet or annoying
        break;
   
      case 'm': 
        ManualState = !ManualState;      // Toggle Voice state, quiet or annoying
        break;

    }    
  
}


void SerialEvent() 
{
  while (Serial.available() && stringComplete == false)    // Read while we have data
  {
    char inChar = Serial.read();             // Read a character
    inData[index] = inChar;                  // Store it in char array
    index++;                                 // Increment where to write next  
    inString += inChar;                      // Also add it to string storage just in case, not used yet :)
    
    if (inChar == '\n' || inChar == '\r')                      // Check for termination character
    {
      index = 0;
      stringComplete = true;
    }
  }
}


