#define trigPinBin D3
#define echoPinBin 14 // D5
#define trigPinBowl D4
#define echoPinBowl 12 //D6
#define red 15 // D8
#define green 16 // D0
#define pompa 13 // D7

#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient client;

String apiKey = "API_KEY";     

const char *ssid = "SSID";
const char *pass = "PASSWORD";
const char* server = "api.thingspeak.com";

int BOWL_PERCENTAGE_FILL = 0;
int BIN_PERCENTAGE_FILL = 0;
int BOWL_MAX_DISTANCE = 200; 
int BOWL_MIN_DISTANCE = 185; 
int BIN_MAX_DISTANCE = 200; 
int BIN_MIN_DISTANCE = 30;

char* freeText = "";

unsigned long PREVIOUS_MILLIS = 0;
long INTERVAL = 10000;
int WATER_BIN_LEVEL = 0;
 
void setup() {
  pinMode(trigPinBin, OUTPUT); 
  pinMode(echoPinBin, INPUT);
  
  pinMode(trigPinBowl, OUTPUT); 
  pinMode(echoPinBowl, INPUT);
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  
  pinMode(pompa, OUTPUT);
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) 
     {
            delay(500);
            Serial.print(".");
     }
   Serial.println("");
   Serial.println("WiFi connected");
}
 
void loop() {  
  unsigned long currentMillis = millis();
  if(currentMillis - PREVIOUS_MILLIS > INTERVAL) {
    PREVIOUS_MILLIS = currentMillis;
    freeText = "woda";
    manageDistance(trigPinBin, echoPinBin, BIN_MIN_DISTANCE, BIN_MAX_DISTANCE, 30, true, "Woda: ", 0,"WaterBin", 1);
    freeText = "miska";
    manageDistance(trigPinBowl, echoPinBowl, BOWL_MIN_DISTANCE, BOWL_MAX_DISTANCE, 10, false, "Miska: ", 1, "WaterBowla", 2);
  }
}

 void sendWifiData(String text, int value, int field) {
  if (client.connect(server,80))   //   "184.106.153.149" or api.thingspeak.com
   {  
      
       String postStr = apiKey;
       postStr +="&field";
       postStr += String(field);
       postStr += "=";
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
       Serial.println(postStr);
  }
  client.stop();
  Serial.println("Waiting...");
 }

int calculatePercentageFill(long minDistance, long maxDistance, long distance) {
  int fill =  ((maxDistance - distance) * 100 / (maxDistance - minDistance));
  Serial.println("FILL");
  Serial.println(fill);
  if(fill > 100) return 100;
  if(fill < 0) return 0;
  return fill;
}

void runPompa(int percentageFill){
  if (percentageFill < 70 && WATER_BIN_LEVEL > 30){
    int seconds = (100 - percentageFill) * 60;
    Serial.println("SECONDS");
    Serial.println(seconds);
    
    pinMode(pompa,OUTPUT);
    unsigned long timeStart = millis();
    digitalWrite(pompa,HIGH);
    delay(seconds);
    unsigned long timeEnd = millis();
    digitalWrite(pompa,LOW);

    Serial.println(timeEnd - timeStart);
  }
}

int calculateFill(int trigPinName, int echoPinName, int minDist, int maxDist) {
  long distance = 0;
  int counter = 0;
  
  while (counter < 10){
    long currentDistance = 0;
    digitalWrite(trigPinName, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPinName, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinName, LOW);
    Serial.println("HERE");
  
    Serial.println(freeText);
    long czas = pulseIn(echoPinName, HIGH);
    Serial.print("PIN ");
    Serial.println(echoPinName);
    currentDistance = czas * 10 / 58;
    Serial.print("DISTANCE ");
    Serial.println(currentDistance);
    distance = distance + currentDistance;
    counter = counter + 1;
    delay(100);
  }

  long avarageDistance = distance / 10;

  Serial.print("AVARAGE DISTANCE ");
    Serial.println(avarageDistance);
    return calculatePercentageFill(minDist, maxDist, avarageDistance);
}
 

void lightBulb (int light, int dark) {
  digitalWrite(light, HIGH); 
  digitalWrite(dark, LOW);
}

void lightLCD (int line, String text) {
  lcd.setCursor(0, line); 
  lcd.print(text); 
  
}

void manageDistance(int trigPinName, int echoPinName, int minDistOfWater, int maxDistOfWater, int lowPercentage, boolean withBulb, String text, int line, String wifiText, int wifiField) {
  int percentageFill = calculateFill(trigPinName, echoPinName, minDistOfWater, maxDistOfWater);
  String output = text + percentageFill + "%";
  if(wifiText == "WaterBin"){
    WATER_BIN_LEVEL = percentageFill;
  }
  lightLCD(line, output);
  sendWifiData(wifiText, percentageFill, wifiField);
  
  if(withBulb){
    lightDioda(percentageFill, lowPercentage);
  }
   if(!withBulb){
    runPompa(percentageFill);
  }
}
 
void lightDioda(long percentageFill, int lowPercentage) {
   Serial.println("BULBO");
    if (percentageFill <= lowPercentage) {
        lightBulb(red, green);
    } else {
        lightBulb(green, red);
    }
}
