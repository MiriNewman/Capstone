#include <LiteOSCParser.h>

#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <QNEthernet.h>

#include "OSC.h"
//#include <LibPrintf.h>

using namespace qindesign::network;

// IP scheme 10.0.0.x
// IP scheme 10.0.10.x for ESP (rainstick, etc)

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

int red = 0;
int green = 0;
int blue = 0;

int colorControl = 0;

bool colorDirectionSwitch = false;


void setup() {
  Serial.begin(9600);
  NetworkBegin();
  
  strip.begin();
  strip.show(); // these reset the LED memory to start afresh

}

/**************************************** BEGIN LOOP ****************************************/
void loop() {
  int size = udp.parsePacket();
  if (0 < size && static_cast<unsigned int>(size) <= sizeof(buf)) {
    udp.read(buf, size);
    printOSC(Serial, buf, size);
  }
  
  for (int i = 0; i < LED_COUNT; i++){ // iterates through the LED strip to give each one a unique color
    pixelSetRGB(i, colorControl);
  }
  
  strip.show(); // actually displays the what's set in PixelSetRGB / setPixelColor();

  colorControl++;
  colorControl %= 255;
  
  delay(50); // set this up to be controllable by a MAX patch
}

/***************************************** END LOOP *****************************************/


/**************************************** BEGIN LEDs ****************************************/

void pixelSetRGB(int pixel, int control){
  /* This function controols the brightness and hues of the LED strip.
   *
   */
  uint16_t hue;
  if (colorDirectionSwitch){
    hue = fmod((control - pixel), 255);
    hue = map(hue, 0, 255, 27000, 60000); // have this be a variable to be set by the MAX patch
  } else {
    hue = fmod((control - pixel), 255);
    hue = map(hue, 0, 255, 60000, 27000);
  }
  
  if ((control == 0) && (colorDirectionSwitch == false)){
    colorDirectionSwitch = true;
    //Serial.println("True");
  } else if ((control == 0) && (colorDirectionSwitch == true)){
    colorDirectionSwitch = false;
    //Serial.println("False");
  }
  uint32_t newColor = strip.ColorHSV(hue, 240, 240);
  
//  if (pixel == 5){
//    Serial.print(control); Serial.print(", "); Serial.print(hue); Serial.print(", "); Serial.println(newColor);
//  }
  strip.setPixelColor(pixel, newColor); // sets pixel color individually
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
    return;
  }

  IPAddress ip = Ethernet.localIP();
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
