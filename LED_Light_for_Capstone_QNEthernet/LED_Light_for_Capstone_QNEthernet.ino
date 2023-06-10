//#include <LiteOSCParser.h>
//::qindesign::osc::LiteOSCParser osc;

#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <QNEthernet.h>

#include "OSC.h"
#include <OSCBundle.h>
//#include <LibPrintf.h>

using namespace qindesign::network;

// IP scheme 192.168.1.[20-24] for clouds

// TEST TEENSY IP: 192.168.1.100

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds
constexpr uint16_t kOSCPort = 8000;
constexpr char kServiceName[] = "Cloud";

EthernetUDP udp;
uint8_t buf[48];

bool ethernetIsConnected = false;

#define LED_PIN 6
#define LED_COUNT 90 //change to however many lights we have
#define ETHERNET_PIN 10

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_BGR + NEO_KHZ800); 
// the lights we have are RGB and use WS2812

uint32_t stormyRGB = strip.Color(20, 50, 50); // red, green, blue
uint32_t whiteRGB = strip.Color(255, 255, 255);
uint32_t lightBlueRGB = strip.Color(200, 200, 255); 
uint32_t medPinkRGB = strip.Color(224, 9, 138);
uint32_t orangeRGB = strip.Color(222, 95, 27);
uint32_t lightFlash = strip.Color(255, 255, 255);
uint32_t allBlack = strip.Color(0, 0, 0);

int colorControl = 0;

/* variables to be controlled by OSC functions at the bottom*/
bool isRaining = false;
bool isThunder = false;
float oscData = 0.0;

/*************************************** BEGIN SETUP ****************************************/
void setup() {
  Serial.begin(9600);
  NetworkBegin(); // RE-ENABLE THIS AFTER LED TESTING IS DONE!!!
  
  strip.begin();
  strip.show(); // these reset the LED memory to start afresh

}

/**************************************** BEGIN LOOP ****************************************/
void loop() {
  //ReadOSC();
  OSCMsgReceive();
  
  for (int i = 0; i < LED_COUNT; i++){ // iterates through the LED strip to give each one a unique color
    pixelSetRGB(i, colorControl, oscData);
  }
  
  strip.show(); // actually displays the what's set in PixelSetRGB / setPixelColor();

  colorControl++;
  colorControl %= 255 * LED_COUNT; // very slow reset
  
  delay(50); // set this up to be controllable by a MAX patch
}

/***************************************** END LOOP *****************************************/


/**************************************** BEGIN LEDs ****************************************/

void pixelSetRGB(int pixel, int control, float loudness){
  /* This function controols the brightness and hues of the LED strip.
   *
   */
  //uint32_t hue;
  float hue = 0;
  int adjustedHue = 0;
  loudness = map(loudness, 0., 1., 0.5, 1.);
  
  hue = ChangeColorChanger(control + (pixel + 1));
  if (pixel == 5 || pixel == 50){
     Serial.print("Loudness in pixelSetRGB is: "); Serial.println(loudness);
     Serial.print(loudness); Serial.print(" || "); Serial.print(hue); Serial.print(" -> ");
     //Serial.println(hue);
  }
  hue *= loudness;

  if (isRaining){
    adjustedHue = map(hue, 0, 255, 27000, 60000); // Set this range to be dimmer / more blue
  } else {
    adjustedHue = map(hue, 0, 255, 27000, 60000); // Set this range to be more yellow/orange/pink
  }
  
  uint32_t newColor = strip.ColorHSV(adjustedHue, 240, 240);

  if (pixel == 5 || pixel == 50){
     Serial.println(hue);
  }
  
  strip.setPixelColor(pixel, newColor);

}

int ChangeColorChanger(int control){
  int colorChanger = abs(255 * sin(control * .01));
  return colorChanger;
}

void LightningStrikeLED(){
  // light flash function for the Lightning Strike. whole operation should take about 1 second

  //private bool lightIsOn = true;
  //private int flashCount = 0; // the maximum flashes will be 3 for  _a e s t h e t i c_
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, lightFlash);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, allBlack);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, lightFlash);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, allBlack);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, lightFlash);
  }
  delay(500);
  isThunder = false;
}

/************************************ BEGIN NETWORK START ***********************************/
void NetworkBegin(){
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  Serial.println("Starting...");

  // Print the MAC address
  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  Serial.printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  // Initialize Ethernet, in this case with DHCP
  Serial.println("Starting Ethernet with DHCP...");
  if (!Ethernet.begin()) {
    Serial.println("Failed to start Ethernet");
    return;
  }
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    Serial.println("Failed to get IP address from DHCP");
    Serial.println("Unplug Teensy from power and try again, then check ethernet and network connections.");
    return;
  }

  IPAddress ip = Ethernet.localIP(); // our Router has reserved IPs for each MAC address
  //IPAddress ip = Ethernet.setLocalIP({192,168,1,10});
  Serial.printf("    Local IP     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  Serial.printf("    Subnet mask  = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.broadcastIP();
  Serial.printf("    Broadcast IP = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  Serial.printf("    Gateway      = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  Serial.printf("    DNS          = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);

  // Listen on port and start an mDNS service
  udp.begin(kOSCPort);
  Serial.println("Starting mDNS...");
  if (!MDNS.begin(kServiceName)) {
    Serial.println("ERROR: Starting mDNS.");
  } else {
    if (!MDNS.addService("_osc", "_udp", kOSCPort)) {
      Serial.println("ERROR: Adding service.");
    } else {
      Serial.printf("Started mDNS service:\r\n"
                    "    Name: %s\r\n"
                    "    Type: _osc._udp\r\n"
                    "    Port: %u\r\n",
                    kServiceName, kOSCPort);
    }
  }
  Serial.println("Waiting for OSC messages...");
}

/************************************** BEGIN OSC SHIT **************************************/
void OSCMsgReceive(){
  OSCMessage msgIN;
  int size;
  if((size = udp.parsePacket())>0){
    while(size--)
      msgIN.fill(udp.read());
    if(!msgIN.hasError()){
      msgIN.route("/intensity", IntensityOSC); //first value is OSC tag, second is function it is calling;
      //Serial.println("Received in Reception, ");
      msgIN.route("/thunder", ThunderOSC);
      msgIN.route("/raining", RainingOSC);
    }
  }
}

void IntensityOSC(OSCMessage &msg, float addrOffset){
  oscData = (float) msg.getFloat(0);
  
  Serial.print("... and in Print! UwU ");
  //Serial.print(msg);
  Serial.println(oscData);
}

void ThunderOSC(OSCMessage &msg, float addrOffset){
  isThunder = true;
  LightningStrikeLED();
}

void RainingOSC(OSCMessage &msg, float addrOffset){
  int rainData = msg.getInt(0);
  if (rainData == 1){
    isRaining = true; // boolean from up top, affects pixelSetRGB;
  } else {
    isRaining == false;
  }
}
