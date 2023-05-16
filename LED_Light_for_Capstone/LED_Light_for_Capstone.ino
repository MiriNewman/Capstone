#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// IP scheme 10.0.0.x
// IP scheme 10.0.10.x for ESP

IPAddress ip(10, 0, 0, 1); // number these sequentially for each cloud controller, 
//REMEMEBER TO CHANGE IT!!!!

#define LED_PIN 6
#define LED_COUNT 90 //change to however many lights we have

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_BGR + NEO_KHZ800); 
// the lights we have are RGB and use WS2812

uint32_t stormyRGB = strip.Color(20, 50, 50); // red, green, blue
uint32_t whiteRGB = strip.Color(255, 255, 255);
uint32_t lightBlueRGB = strip.Color(200, 200, 255); 
uint32_t medPinkRGB = strip.Color(224, 9, 138);
uint32_t orangeRGB = strip.Color(222, 95, 27);

int colorControl = 0;

//enum colorWheel[] = {stormyRGB, whiteRGB, lightBlueRGB, medPinkRGB, orangeRGB};

void setup() {
  Serial.begin(9600);
  
  strip.begin();
  strip.show();
  //Serial.print(strip.read()); // someone figure something out for this
  
}

void loop() {
  int randomBrightness = random(100);
  //int px = 0;
  //strip.setPixelColor(n, red, green, blue); // pseudocode example

  for (int i = 0; i < LED_COUNT; i++){
    pixelSetRGB(i, colorControl);
  }

//  for (int i = 0; i < LED_COUNT; i++){
//    if (i % 5 == 0){
//      pixelSet(i, orangeRGB);
//    } else if (i % 8 == 0){
//      pixelSet(i, medPinkRGB);
//    } else {
//      pixelSet(i, stormyRGB);
//    }
//  }
  //strip.setBrightness(150 + randomBrightness);
  strip.show();

  Serial.println(colorControl);

  colorControl++;
  colorControl %= 255;
  delay(500);
}

void pixelSet(int pixel, int color){
  strip.setPixelColor(pixel, color);
}

void pixelSetRGB(int pixel, int control){ //, int r, int g, int b){
  float r = fmod(1 + pixel /*+ 2.8 * pixel*/ + control, 255);
  float g = fmod(120 + pixel/*+ 2.8 * pixel*/ + control, 255);
  float b = fmod(240 + pixel /*2.8 * pixel*/ + control, 255);
//  float r = fmod(1  +  pixel * 2 + control, 255);
//  float g = fmod(220 + pixel * 2 + control, 255);
//  float b = fmod(250 + pixel * 2 + control, 255);
//  float r = (pixel + control) % 255;
//  float g = (120 + pixel + control) % 255;
//  float b = (240 + pixel + control) % 255;
  if (pixel == 5){
    Serial.print("red: ");
    Serial.println(r);
    Serial.print("green: ");
    Serial.println(g);
    Serial.print("blue: ");
    Serial.println(b);
  }
  strip.setPixelColor(pixel, r, g, b);
}
