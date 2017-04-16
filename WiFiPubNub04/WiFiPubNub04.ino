/*
 *  Huzzah Freather - PubNub Example
 *
 */

#include <ESP8266WiFi.h>
#define PubNub_BASE_CLIENT WiFiClient
#define PUBNUB_DEFINE_STRSPN_AND_STRNCASECMP
#include <PubNub.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <ArduinoJson.h>

#define TBUTTON 12

const char* ssid     = "your ssid here";
const char* password = "your wifi here";

class ADC
{
  // state variables
  long currentMillis, previousMillis;
  long colorInterval;

  uint16_t red, green, blue;
  uint8_t oldRed, oldGreen, oldBlue;
  uint8_t mappedRed, mappedGreen, mappedBlue;
  
  boolean colorChanged;

  char jsonPayload[100];
  char eonPayload[150];
    
  public:

    Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
    
    boolean getColorChanged() { return colorChanged; }

    char* getJsonPayload() { return jsonPayload; }
    char* getEonPayload() {return eonPayload; }

    ADC(int dummy){}
    
    void init()
    {
      currentMillis = 0;
      previousMillis = 0;
      colorInterval = 60; // adafruit colorview example uses delay(60) function
      // initial color values
      red = 0;
      green = 0;
      blue = 0;
      ads.begin();
      colorChanged = false;
      createJsonPayload();
      createEonPayload();
    }
    
    void Update()
    {  
      currentMillis = millis();
      if( (currentMillis - previousMillis) > 250) 
      {  
        red = ads.readADC_SingleEnded(0);
        green = ads.readADC_SingleEnded(1);
        blue =  ads.readADC_SingleEnded(2);
        // map(value, fromLow, fromHigh, toLow, toHigh)
        mappedRed = map(red, 0, 1095, 0, 255);
        mappedGreen = map(green, 0, 1095, 0, 255);
        mappedBlue = map(blue, 0, 1095, 0, 255);   
        colorChange(mappedRed, mappedGreen, mappedBlue);
      }
    }

    void colorChange( uint8_t currRed, uint8_t currGreen, uint8_t currBlue)
    {
      
      int deltaRed = currRed-oldRed;
      int deltaGreen = currGreen-oldGreen;
      int deltaBlue = currBlue-oldBlue;

      if ( abs(deltaRed) > 3 || abs(deltaGreen) >3 || abs(deltaBlue) > 3) {
          //debugging
          Serial.println("color change detected");
          createJsonPayload();
          createEonPayload();
          colorChanged = true;
          oldRed = currRed;
          oldGreen = currGreen;
          oldBlue = currBlue;
      }
      else {colorChanged = false;}
    }

    void createJsonPayload()
    {
      StaticJsonBuffer<100> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["R"] = mappedRed;
      root["G"] = mappedGreen;
      root["B"] = mappedBlue;
      root.printTo(jsonPayload,sizeof(jsonPayload));
    }

    void createEonPayload()
    {
      StaticJsonBuffer<150> jsonBuffer;
      JsonObject& colorValues = jsonBuffer.createObject();
      JsonObject& nestedColorValues = colorValues.createNestedObject("eon");
      nestedColorValues["R"] = mappedRed;
      nestedColorValues["G"] = mappedGreen;
      nestedColorValues["B"] = mappedBlue;
      colorValues.printTo(eonPayload, sizeof(eonPayload));
    }
};

ADC adc(1);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(TBUTTON, INPUT_PULLUP);
  PubNub.begin("your publish key here", "your subcribe key here");
  adc.init();    
}

int value = 0;

long currentPublishMillis;
long previousPublishMillis;

void loop() {
  if ( digitalRead(TBUTTON) == LOW ) { Serial.println("TBUTTON PRESSED"); }
  // Use WiFiClient class to create TCP connections
  WiFiClient *client;
  adc.Update(); 
  if (adc.getColorChanged()) {
    client = PubNub.publish("HuzzahChannel", adc.getEonPayload());
  }
}


  
  
  

