

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.
// Color definitions
#define SCLK_PIN 13
#define MOSI_PIN 11
#define DC_PIN   8
#define CS_PIN   10
#define RST_PIN  9
#define  BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
static const int RXPin = 6, TXPin = 7;
const int GPSBaud=9600;
int previous=0;;
// The TinyGPS++ object
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);




uint16_t time;

void setup(void) 
{
  Serial.begin(9600);
  Serial.print("hello!");
  tft.begin();
ss.begin(GPSBaud);
  Serial.println("init");
     delay(6000);
   tft.fillScreen(BLACK);
  tft.setCursor(30, 5);
  tft.setTextColor(YELLOW);
  tft.setTextSize(3);
  tft.println("GPS");
   tft.println("Speedo");
   tft.println("meter");
  delay(5000);
  tft.setTextColor(BLUE);
  tft.setTextSize(1);
  tft.setCursor(0, 10);
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.println("Loading..");
  
  delay(5000);
  
  tft.setTextColor(GREEN);
  tft.setTextSize(2);
  tft.setCursor(0, 25);
  tft.println("Please");
  tft.println("Wait....");
  delay(5000);
  //tft.fillRect(0, 0, 128, 128, BLACK);
   tft.fillScreen(BLACK);
    tft.setTextSize(2);
     tft.setTextColor(BLUE);
}

void loop()
{

  bool newdata = false;
  unsigned long start = millis();

  // Every 5 seconds we print an update
  while (millis() - start < 20)
  {
    if (ss.available()) 
    {
      char c = ss.read();
      // Serial.print(c);  // uncomment to see raw GPS data
      if (gps.encode(c)) 
      {
        newdata = true;
       
      }
    }
  }
  
  if (newdata)
  {
   
    Serial.println(int(gps.speed.kmph()));   
    tft.setCursor(0,0);
    tft.setTextColor(BLUE);
     tft.setTextSize(3);
      tft.print("Speed:"); 
      tft.setCursor(50,45);
      tft.setTextSize(5);
      tft.setTextColor(GREEN); 
      //tft.fillScreen(BLACK) ;
       if(previous!=int(gps.speed.kmph()))
         {
         clear_text();
         }    
      tft.print(int(gps.speed.kmph()));
         previous=int(gps.speed.kmph());
        
      tft.setCursor(10,100);
      tft.setTextSize(3);
      tft.setTextColor(CYAN);
      tft.print(" km/h ");  
      
     
    
       
        
  
  
}
}
  void testfillcircles(uint8_t radius, uint16_t color)
  {
  for (uint8_t x=radius; x < tft.width()-1; x+=radius*2) {
    for (uint8_t y=radius; y < tft.height()-1; y+=radius*2) {
      tft.fillCircle(x, y, radius, color);
    }
  }  
}
 void clear_text()
 {
  for (int y=35; y<=85; y++)
      {
       for (int x=0; x<127; x++)
       {
        tft.drawPixel(x, y, BLACK); 
       }
       
     }
 } 
