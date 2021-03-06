/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TM1637Display.h>
#include <DHT.h>;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "8ERyFvLfBmyH8Sz3q_q7_rnORwhlYK2Z";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "VAIO_Network";
char pass[] = "imda-z76m-ylz9";
WidgetLED ledMotorOn(V2);  
WidgetLED ledMotorOff(V3);
WidgetLED ledUp(V7);
float temp;
float setTemp;
boolean motorOn;

//Constants
#define DHTPIN 13    // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define CLK 14
#define DIO 12

DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
BlynkTimer timer;
TM1637Display display = TM1637Display(CLK, DIO);

// Create array that turns all segments on:
const uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
// Create array that turns all segments off:
const uint8_t blank[] = {0x00, 0x00, 0x00, 0x00};
// You can set the individual segments per digit to spell words or create other symbols:
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};
// Create degree Celsius symbol:
const uint8_t celsius[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,  // Circle
  SEG_A | SEG_D | SEG_E | SEG_F   // C
};

const int inUp = 16;
const int relayPin = 4;
// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  temp = t;
  Blynk.virtualWrite(V6, h);
  Blynk.virtualWrite(V1, temp);
  
  BLYNK_LOG("Temperatura lida: %s", t);
     if(temp>setTemp)
     {
       if (!motorOn){
         BLYNK_LOG("Motor ligou");
         ledMotorOn.on();
         ledMotorOff.off();
         motorOn = true;
         digitalWrite(relayPin, HIGH);// turn on Fridge Compressor
       }
     }

     int setTempDelta = setTemp - 3;
     if (temp < setTempDelta) {  
       if (motorOn){
          BLYNK_LOG("Motor desligou");
          ledMotorOn.off();
          ledMotorOff.on();
          motorOn = false;
          digitalWrite(relayPin, LOW);
       }
     }
  //display.showNumberDec(temp, true); // Prints displaytime without center colon.
  display.showNumberDec(temp, false, 2, 0);
  display.setSegments(celsius, 2, 2);
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  pinMode(inUp, INPUT);
  pinMode(relayPin, OUTPUT);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  setTemp = 10;
  Blynk.virtualWrite(0, setTemp);// 10 Geladeira - 18 Adega

  dht.begin();
  
  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);

  // Set the display brightness (0-7):
  display.setBrightness(3);
  // Clear the display:
  display.clear();
  motorOn = false;
}

void updateDisplay(float t){
  delay(500);
  display.clear();
  display.showNumberDec(t, false, 2, 1);
  delay(1000);
  display.setSegments(done);
  delay(1000);
}

BLYNK_WRITE(0)
{
  BLYNK_LOG("Got a value: %s", param.asStr());

  setTemp=param.asFloat();
  updateDisplay(setTemp);
  }


//BLYNK_WRITE(V5) // Get and store push button status 
//{
//    int frigeType = param.asInt(); // save status
//    if(frigeType==1) 
//    {//adega
//      setTemp=18;
//    }else{//geladeira
//      setTemp=10; 
//    }
//    //updateDisplay();
//    Blynk.virtualWrite(0, setTemp);// 10 Geladeira - 18 Adega
//}

void loop()
{
  int val = digitalRead(inUp);
  if (val == HIGH){
    ledUp.on();
    setTemp = setTemp + 1;
    Blynk.virtualWrite(V0, setTemp);// 10 Geladeira - 18 Adega
    updateDisplay(setTemp);   
  }else{
    ledUp.off();
  }
  
  Blynk.run();
  timer.run();
}
