#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Wire.h>
#include "SSD1306.h"
#include "SSD1306Ui.h"
#include "images.h"
#define ONE_WIRE_BUS            D1      // DS18B20 pin

const char* ssid = "ssid";
const char* password = "password";

ESP8266WebServer server(80);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char temperatureString[6];
const int led = BUILTIN_LED;
char ip[16];
unsigned long nowTime=0;
unsigned long lastTime=0;
extern float getTemperature();

// Initialize the oled display for address 0x3c
//SSD1306(int i2cAddress, int sda, int sdc)
SSD1306   display(0x3c, D3, D4);
SSD1306Ui ui     ( &display );
String ipAddress;
// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
extern bool drawFrame1(SSD1306 *display, SSD1306UiState* state, int x, int y);
extern bool drawFrame2(SSD1306 *display, SSD1306UiState* state, int x, int y);
extern bool drawFrame3(SSD1306 *display, SSD1306UiState* state, int x, int y);
extern bool drawFrame4(SSD1306 *display, SSD1306UiState* state, int x, int y);
extern bool msOverlay(SSD1306 *display, SSD1306UiState* state);

bool (*frames[])(SSD1306 *display, SSD1306UiState* state, int x, int y) = { drawFrame1, drawFrame2, drawFrame3, drawFrame4 };

// how many frames are there?
int frameCount = 4;

bool (*overlays[])(SSD1306 *display, SSD1306UiState* state)             = { msOverlay };
int overlaysCount = 1;

void displayTemperature(String output)
{
  display.clear();
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(20, 10, output);
  
  display.drawString(20, 30,ip);
  display.display();
}

void handleRoot() {
  digitalWrite(led, LOW);

  float temperature = getTemperature();
  // convert temperature to a string with two digits before the comma and 2 digits for precision
  dtostrf(temperature, 2, 2, temperatureString);
   Serial.println(temperatureString );
   
String temp = temperatureString;
String output = "current temp: "+ temp;
  server.send(200, "text/plain",output );
  Serial.println("we receive a request");
  digitalWrite(led, HIGH);

   displayTemperature(output);
   lastTime = millis();
}

void handleNotFound(){
  digitalWrite(led, LOW);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, HIGH);
}

float getTemperature() {
  Serial.print("requesting");
  float temp;
  do {
    DS18B20.requestTemperatures();     
    temp = DS18B20.getTempCByIndex(0);     
    //delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

void setup(void){

/*
 ui.setTargetFPS(5);

  ui.setActiveSymbole(activeSymbole);
  ui.setInactiveSymbole(inactiveSymbole);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Inital UI takes care of initalising the display too.
  ui.init();

  display.flipScreenVertically();
  */
//  ui.init();
  //display.sendInitCommands();
display.init();
display.flipScreenVertically();
display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.displayOn();
    
   // setup OneWire bus
  DS18B20.begin();
  
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  
  IPAddress ipAddress=WiFi.localIP();
  sprintf(ip,"%d:%d:%d:%d", ipAddress[0],ipAddress[1],ipAddress[2],ipAddress[3]); 
  Serial.println(ipAddress);

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}


void loop(void){
  nowTime = millis();//current time
   server.handleClient();//always to prevent delays
   
  if(nowTime-lastTime>1000)
  {
     float temperature = getTemperature();
    // convert temperature to a string with two digits before the comma and 2 digits for precision
    dtostrf(temperature, 2, 2, temperatureString);
    Serial.println(temperatureString );
    String temp = temperatureString;
    String output = "current temp: "+ temp;
    displayTemperature(output);
  }
  else if(nowTime-lastTime<0)
    lastTime=0;
   /*
int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    server.handleClient();
    delay(remainingTimeBudget);
  }  */
}

bool msOverlay(SSD1306 *display, SSD1306UiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
  return true;
}

bool drawFrame1(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  // if this frame need to be refreshed at the targetFPS you need to
  // return true
  display->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  return false;
}

bool drawFrame2(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, "Arial 10");

  display->setFont(ArialMT_Plain_16);
  display->drawString(0 + x, 20 + y, "Arial 16");

  display->setFont(ArialMT_Plain_24);
  display->drawString(0 + x, 34 + y, "Arial 24");

  return false;
}

bool drawFrame3(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // Text alignment demo
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33, "Right aligned (128,33)");
  return false;
}

bool drawFrame4(SSD1306 *display, SSD1306UiState* state, int x, int y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
  return false;
}

