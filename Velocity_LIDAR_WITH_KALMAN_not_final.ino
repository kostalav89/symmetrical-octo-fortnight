 #include <LIDARLite.h>
#include <Wire.h> //Used for I2C
#include <avr/wdt.h> //We need watch dog for this program
#include "KalmanFilter.h"
#define    LIDARLite_ADDRESS   0x62          // Default I2C Address of LIDAR-Lite.
#define    RegisterMeasure     0x00          // Register to write to initiate ranging.
#define    MeasureValue        0x04          // Value to initiate ranging.
#define    RegisterHighLowB    0x8F          // Register to get both High and Low bytes in 1 call.
LIDARLite myLidarLite;
KalmanFilter kalmanFilter;
//GPIO declarations
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
byte en_LIDAR = A0; //Low makes LIDAR go to sleep, high is normal operation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long lastTime = 0;
long lastReading = 0;
int lastDistance = 0;
float newDistance;
const byte numberOfDeltas = 8;
float deltas[numberOfDeltas];
byte deltaSpot = 0; //Keeps track of where we are within the deltas array
#define LOOPTIME 50
void petFriendlyDelay(int timeMS)
{
  long current = millis();
  
  while(millis() - current < timeMS)
  {
    delay(1);
    wdt_reset(); //Pet the dog
  }
}

//Get a new reading from the distance sensor
int readLIDAR(void)
{
  int distance = 0;

  Wire.beginTransmission((int)LIDARLite_ADDRESS); // transmit to LIDAR-Lite
  Wire.write((int)RegisterMeasure); // sets register pointer to  (0x00)
  Wire.write((int)MeasureValue); // sets register pointer to  (0x00)
  Wire.endTransmission(); // stop transmitting 

  delay(20); // Wait 20ms for transmit
  wdt_reset(); //Pet the dog     

  Wire.beginTransmission((int)LIDARLite_ADDRESS); // transmit to LIDAR-Lite
  Wire.write((int)RegisterHighLowB); // sets register pointer to (0x8f)
  Wire.endTransmission(); // stop transmitting

  delay(20); // Wait 20ms for transmit
  wdt_reset(); //Pet the dog

  Wire.requestFrom((int)LIDARLite_ADDRESS, 2); // request 2 bytes from LIDAR-Lite

  if (Wire.available() >= 2) // if two bytes were received
  {
    distance = Wire.read(); // receive high byte (overwrites previous reading)
    distance = distance << 8; // shift high byte to be high 8 bits
    distance |= Wire.read(); // receive low byte as lower 8 bits
    return (distance);
  }
  else
  {
    Serial.println("Read fail");
    disableLIDAR();
    delay(100);
    enableLIDAR();

    return(0);
  }
}
void disableLIDAR()
{
  digitalWrite(en_LIDAR, LOW);
}

void enableLIDAR()
{
  digitalWrite(en_LIDAR, HIGH);  
}
void Kilman_output(float standard_val)
  {
    
      kalmanFilter.correct(standard_val); 
      double correctedValue=kalmanFilter.getState();  
      Serial.println(correctedValue);
  }
  


void setup()
{
  wdt_reset(); //Pet the dog
  wdt_disable(); //We don't want the watchdog during init

  Serial.begin(9600);
  Serial.println("CLEARDATA");
  Serial.println("LABEL, current time, current value,correction value");
  Wire.begin();
  myLidarLite.begin(0, true); 
  myLidarLite.configure(3); // Change this number to try 
  pinMode(en_LIDAR, OUTPUT);
  
  Serial.println("Coming online");

  enableLIDAR();
  while(readLIDAR() == 0)
  {
    Serial.println("Failed LIDAR read");
    delay(100);
  }

  wdt_reset(); //Pet the dog
  wdt_enable(WDTO_250MS); //Unleash the beast

   //kalmanFilter.setState( readLIDAR() );
   //kalmanFilter.setState( instantKMH );
}

void loop()
{
  wdt_reset(); //Pet the dog
   if (millis() - lastReading > (LOOPTIME-1)) // 49)
  {
    lastReading = millis();

    //Every loop let's get a reading
    newDistance = readLIDAR(); //Go get distance in cm   
    //Error checking
    if(newDistance > 3500) newDistance = 0;
    
    int deltaDistance = lastDistance - newDistance;
    lastDistance = newDistance;

     boolean safeDelta = true;
    for(int x = 0 ; x < numberOfDeltas ; x++)
    {
      //We don't want to register jumps greater than 70cm in 50ms it means that the speed will be 
      //But if we're less than 1000cm then maybe
      //30 works well
      //if( abs(deltaDistance - deltas[x]) > 30 || deltaDistance >170) 
        if( abs(deltaDistance - deltas[x]) > 70) 
           safeDelta = false; 
    }  
    
    //Insert this new delta into the array
    if(safeDelta)
    {
      deltas[deltaSpot++] = deltaDistance;
      if (deltaSpot > numberOfDeltas) 
         deltaSpot = 0; //Wrap this variable
    }
    float avgDeltas = 0.0;
    for (byte x = 0 ; x < numberOfDeltas ; x++)
      avgDeltas += (float)deltas[x];
    avgDeltas /= numberOfDeltas;
     float instantKMH = 0.6215*22.36936 * (float)avgDeltas / (float)LOOPTIME ;
     instantKMH=int(ceil(instantKMH)); //Round up to the next number. This is helpful if we're not displaying decimals.
   
   
       
   
     Serial.println(newDistance);
     Kilman_output(instantKMH);
     
 

}
}

  
