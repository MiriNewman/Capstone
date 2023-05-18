//#include <NativeEthernet.h>
//#include <SPI.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>
#include <QNEthernet.h>
#include <TimeLib.h>
#include <LibPrintf.h>

using namespace qindesign::network;

// IP scheme 10.0.0.x
// IP scheme 10.0.10.x for ESP (rainstick, etc)

constexpr uint32_t kDHCPTimeout = 15000;
constexpr uint16_t kNTPPort = 123;

constexpr uint32_t kEpochDiff = 2'208'988'800;
constexpr uint32_t kBreakTime =2'085'978'496;

EthernetUDP udp;
uint8_t buf[48];

//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
//char server[] = "www.arduino.cc";
//IPAddress ip(10, 0, 0, 1); // number these sequentially for each cloud controller, 
////REMEMEBER TO CHANGE IT!!!!
//IPAddress myDNS(10, 10, 0, 1); // ??
//
//EthernetClient client;
//
//unsigned long beginMicros, endMicros;
//unsigned long byteCount = 0;
//unsigned long lastConnectionTime = 0;
//const unsigned long postingInterval = 10*1000;
//bool printWebData = true; // something to do with speed measurement?


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

int colorControl = 0;

bool colorDirectionSwitch = false;

void setup() {
  Serial.begin(9600);
  //Ethernet.begin();

//  Ethernet.init(ETHERNET_PIN);
//  
  strip.begin();
  strip.show(); // these reset the LED memory to start afresh

//  Serial.println("Initialize Ethernet with DHCP:");
//  if (Ethernet.begin(mac) == 0){
//    Serial.println("Failed to configure Ethernet using DHCP");
//    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
//      Serial.println("Ethernet shield was not found. Check physical connection or replace Teensy. <3");
//      while (true) {
//      }
//    }
//    if (Ethernet.linkStatus() == LinkOFF) {
//      Serial.println("Ethernet cable is not connected.");
//    }
//    Ethernet.begin(mac, ip, myDNS);
//    Serial.print("My IP address: ");
//    Serial.println(Ethernet.localIP());
//  } else {
//    Serial.print(" DHCP assigned IP ");
//    Serial.println(Ethernet.localIP());
//  }
//  delay(1000); // giving ethernet shield a minut to initialize
//  Serial.print ("Connecting to "); Serial.print(server); Serial.println(". . .");
//
//  if (client.connect(server, 80)) {
//    Serial.print("Connected to "); Serial.println(client.remoteIP());
//    client.println("GET /search?q=arduino HTTP/1.1");
//    client.println("Host: www.google.com");
//    client.println("Connection: close");
//    client.println();
//  } else {
//    Serial.println("Connection failed. :-(");
//  }
//  beginMicros = micros();

  while (!Serial && millis() < 4000) {
    // waiting for monitor
  }
  printf("Starting...\r\n");

  uint8_t mac[6];
  Ethernet.macAddress(mac);
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printf("Starting Ethernet with DCHP...\r\n");
  Ethernet.onLinkState([](bool state){
    printf("[Ethernet] Link %s\n", state ? "ON" : "OFF");
  });
  if(!Ethernet.begin()) {
    printf("Failed to start Ethernet\r\n");
    return;
  }
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    printf("Failed to get IP address from DHCP\r\n");
    return;
  }
  //IPAddress ip = (10,0,0,1);
  IPAddress ip = Ethernet.localIP();
  printf("    Local IP    = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  printf("    Subnet mask = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  printf("    Gateway     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  printf("    DNS         = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);

  udp.begin(kNTPPort);

  memset(buf, 0, 48);
  buf[0] = 0b00'100'011; // LI = 0, VN = 4, Mode = 3 (Client) // ??
  uint32_t t = Teensy3Clock.get();
  if (t <= kBreakTime) {
    t -= kBreakTime;
  } else {
    t += kEpochDiff;
  }

  buf[40] = t >> 24;
  buf[41] = t >> 16;
  buf[42] = t >> 8;
  buf[43] = t;

  printf("Sending SNTP request to the gateway. . .");
  if (udp.send(Ethernet.gatewayIP(), kNTPPort, buf, 48)) {
    printf("ERROR");
  }
  printf("\r\n");

  Serial.println("If you read this before anything else, someone is bad at writing libraries.");
}

void loop() {
  for (int i = 0; i < LED_COUNT; i++){ // iterates through the LED strip to give each one a unique color
    pixelSetRGB(i, colorControl);
  }
  
  strip.show(); // actually displays the what's set in PixelSetRGB / setPixelColor();

  colorControl++;
  colorControl %= 255;
  if ((colorControl == 0) && (colorDirectionSwitch == false)){
    colorDirectionSwitch = true;
    //Serial.println("True");
  } else if ((colorControl == 0) && (colorDirectionSwitch == true)){
    colorDirectionSwitch = false;
    //Serial.println("False");
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
  
//  if (pixel == 5){
//    Serial.print(control); Serial.print(", "); Serial.print(hue); Serial.print(", "); Serial.println(newColor);
//  }
  strip.setPixelColor(pixel, newColor); // sets pixel color individually
}
