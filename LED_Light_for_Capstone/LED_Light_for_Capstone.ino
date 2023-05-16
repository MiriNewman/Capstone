#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

// IP scheme 10.0.0.x
// IP scheme 10.0.10.x for ESP (rainstick, etc)

IPAddress ip(10, 0, 0, 1); // number these sequentially for each cloud controller, 
//REMEMEBER TO CHANGE IT!!!!

EthernetClient client;

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

bool colorDirectionSwitch = false;

void setup() {
  Serial.begin(9600);
  
  strip.begin();
  strip.show(); // these reset the LED memory to start afresh
  
}

void loop() {
  //int randomBrightness = random(100);

  for (int i = 0; i < LED_COUNT; i++){ // iterates through the LED strip to give each one a unique color
    pixelSetRGB(i, colorControl);
  }
  
  strip.show(); // actually displays the what's set in PixelSetRGB / setPixelColor();

  //Serial.println(colorControl);

  colorControl++;
  colorControl %= 255;
  if ((colorControl == 0) && (colorDirectionSwitch == false)){
    colorDirectionSwitch = true;
    Serial.println("True");
  } else if ((colorControl == 0) && (colorDirectionSwitch == true)){
    colorDirectionSwitch = false;
    Serial.println("False");
  }
  
  delay(50); // set this up to be controllable by a MAX patch
}

void pixelSet(int pixel, int color){ // deprecated function as of 5/16/23
  strip.setPixelColor(pixel, color);
}

void pixelSetRGB(int pixel, int control){
  //uint16_t hue = strip.Color(control + pixel, control - pixel, control);
  uint16_t hue;
  if (colorDirectionSwitch){
    hue = fmod((control - pixel), 255);
    hue = map(hue, 0, 255, 27000, 60000); // have this be a variable to be set by the MAX patch
  } else {
    hue = fmod((control - pixel), 255);
    hue = map(hue, 0, 255, 60000, 27000);
  }
  // BUILD A BOOLEAN STATEMENT TO HAVE THINGS RAMP BACK AND FORTH
  uint32_t newColor = strip.ColorHSV(hue, 240, 240);
  
  if (pixel == 5){
    Serial.print(control); Serial.print(", "); Serial.print(hue); Serial.print(", "); Serial.println(newColor);
  }
  strip.setPixelColor(pixel, newColor); // sets pixel color individually
}
