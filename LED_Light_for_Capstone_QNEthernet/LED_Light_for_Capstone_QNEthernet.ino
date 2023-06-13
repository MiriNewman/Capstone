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
#define LED_COUNT 65 //change to however many lights we have
#define ETHERNET_PIN 10

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_BGR + NEO_KHZ800); 
// the lights we have are RGB and use WS2812

uint32_t stormyRGB = strip.Color(20, 50, 50); // red, green, blue
int lightBlueRGB[] = {255, 200, 200};
int yellowRGB[] = {255, 230, 255};
int sunnyRGB[] = {50, 255, 150}; // blue, red, green
int rainyRGB[] = {150, 90, 50};
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
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 0, 0, 0);
  }
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
  uint32_t hue = strip.Color(0, 0 , 0);
  int adjustedHue = 0;
  loudness = map(loudness, 0., 1., 0.5, 1.);
  
  //hue = ChangeColorChanger(control + (pixel + 1));
  hue = 0;

  if (isRaining){
    hue = strip.Color(rainyRGB[0] * loudness,
                      rainyRGB[1] * loudness,
                      rainyRGB[2] * loudness);
    //adjustedHue = map(hue, 8000000, 12000000, 0, 60000);
  } else {
    hue = strip.Color(sunnyRGB[0] * loudness, 
                      sunnyRGB[1] * loudness, 
                      sunnyRGB[2] * loudness);
  }
//  if (pixel == 5){
//    Serial.print("Hue: "); 
//    if (isRaining){
//      Serial.print(lightBlueRGB[0]); Serial.print(lightBlueRGB[1]); Serial.println(lightBlueRGB[2]);
//    } else {
//      Serial.print(yellowRGB[0]); Serial.print(yellowRGB[1]); Serial.println(yellowRGB[2]);
//    }
//  }
  
  //uint32_t newColor = strip.ColorHSV(adjustedHue, 240, 240);
  uint32_t newColor = hue;
  
  strip.setPixelColor(pixel, newColor);

}

int ChangeColorChanger(int control){
  int colorChanger = abs(255 * sin(control * .01));
  return colorChanger;
}

void LightningStrikeLED(){
  // light flash function for the Lightning Strike. whole operation should take about 1 second
  
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 255, 255, 255);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 0, 0, 0);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 255, 255, 255);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 0, 0, 0);
  }
  delay(125);
  for (int i = 0; i < LED_COUNT; i++){
    strip.setPixelColor(i, 255, 255, 255);
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
      msgIN.route("/thunder", ThunderOSC);
      msgIN.route("/raining", RainingOSC);
    }
  }
}

void IntensityOSC(OSCMessage &msg, float addrOffset){
  oscData = (float) msg.getFloat(0);
}

void ThunderOSC(OSCMessage &msg, float addrOffset){
  isThunder = true;
  Serial.println("Thunder!");
  LightningStrikeLED();
}

void RainingOSC(OSCMessage &msg, float addrOffset){
  int rainData = msg.getInt(0);
  if (rainData == true){
    isRaining = true; // boolean from up top, affects pixelSetRGB;
    Serial.println("Rain has started.");
  } else {
    isRaining = false;
    Serial.println("The rain has ended.");
  }
}
