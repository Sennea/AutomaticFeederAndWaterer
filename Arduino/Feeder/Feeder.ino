#include "HX711.h"
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

String apiKey = "API_KEY";    
const char *ssid = "SSID";
const char *pass = "PASS";
const char* server = "api.thingspeak.com";


const int SQUEEZE_PIN = A0;
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 14;
const int SERVO = 13;
const int RA9P = A0;
#define red 16 // D0
#define green 15 // D8
HX711 scale;
WiFiClient client;

const float MIN_WEIGHT = 0;
const float MAX_WEIGHT = 55;

const float MIN_BOX_WEIGHT = 75;
const float MAX_BOX_WEIGHT = 3000;

unsigned long PREVIOUS_MILLIS = 0;
long INTERVAL = 10000;

void setup() {
  Serial.begin(38400);
  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.read();
  scale.read_average(20);
  scale.get_value(5);
  scale.get_units(5);
  scale.tare();

  scale.set_scale(2200.f);
  scale.read();
  scale.read_average(20);
  scale.get_value(5);
  scale.get_units(5);

  Serial.print("Odczyt wagi:");
  Serial.println(scale.get_units(5));

  servo.attach(13, 544, 2400);
  servo.write(0);
  lcd.init();
  lcd.backlight();
  delay(1000);
  //  servo.write(70);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) 
     {
            delay(500);
            Serial.print(".");
     }
   Serial.println("");
   Serial.println("WiFi connected");
}

int calculatePercentageFill(long minDistance, long maxDistance, long distance) {
  int fill =  ((distance - minDistance) * 100 / (maxDistance - minDistance));
  if (fill > 100) return 100;
  if (fill < 0) return 0;
  return fill;
}

 void sendWifiData(String text, int value, int field) {
  if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
   {  
       String postStr = apiKey;
       if(field == 3) {
        postStr +="&field3=";
       }else {
        postStr +="&field4=";
       }
       postStr += String(value);
       postStr += "\r\n\r\n";

       client.print("POST /update HTTP/1.1\n");
       client.print("Host: api.thingspeak.com\n");
       client.print("Connection: close\n");
       client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
       client.print("Content-Type: application/x-www-form-urlencoded\n");
       client.print("Content-Length: ");
       client.print(postStr.length());
       client.print("\n\n");
       client.print(postStr);

       Serial.print(text);
       Serial.print(value);
       Serial.println("%. Send to Thingspeak.");
  }
  client.stop();

  Serial.println("Waiting...");
 }


void runServo(float percentageWeight) {
  int seconds = (100 - percentageWeight) * 60 / 6;
  Serial.print("Seconds");
  Serial.println(seconds);
  if (seconds > 0) {
    servo.write(170);
    delay(seconds);
    servo.write(0);
  }
}


void adjustBowlFill() {
  int weight = scale.get_units(5);

  int bowlPercentageFill = calculatePercentageFill(MIN_WEIGHT, MAX_WEIGHT, weight);
  sendWifiData("foodBowlFill", bowlPercentageFill, 4);
  String output = "Bowl:" + String(bowlPercentageFill) + "%...";

  lightLCD(1, output);
  if (bowlPercentageFill < 30) {
    runServo(bowlPercentageFill);
  }
}

void setBowlFillOnLcd() {
  String output = "Bowl:" + String(calculatePercentageFill(MIN_WEIGHT, MAX_WEIGHT, scale.get_units(5))) + "%...";
    sendWifiData("foodBowlFill", calculatePercentageFill(MIN_WEIGHT, MAX_WEIGHT, scale.get_units(5)), 4);
  lightLCD(1, output);
}

int getBoxFill() {
  float squeeze = analogRead(SQUEEZE_PIN);
  Serial.println(squeeze);
  Serial.println(map(squeeze, 0, 1024, 0, 255));
  int formatedBoxFill = map(squeeze, 0, 1024, 0, 255);

    sendWifiData("fodBinFill", formatedBoxFill, 3);
    String output = "BoxFill:" + String(formatedBoxFill) + "g...";
    lightLCD(0, output);
    
  return map(squeeze, 0, 1024, 0, 255);
}


void lightBulb (int light, int dark) {
  digitalWrite(light, HIGH);
  digitalWrite(dark, LOW);
}

void lightLCD (int line, String text) {
  lcd.setCursor(0, line);
  lcd.print(text);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - PREVIOUS_MILLIS > INTERVAL) {
    PREVIOUS_MILLIS = currentMillis;
      lightBulb(green, red);

    int boxFill = getBoxFill();
    if (boxFill > MIN_BOX_WEIGHT ) {
      adjustBowlFill();
      lightBulb(green, red);
    } else {
      setBowlFillOnLcd();
      lightBulb(red, green);
    }
  }
}
