#include <QNEthernet.h>

#include "OSC.h"
#include <OSCBundle.h>
using namespace qindesign::network;

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds
constexpr uint16_t kOSCPort = 8000;
constexpr char kServiceName[] = "Cloud";

EthernetUDP udp;
uint8_t buf[48];

bool ethernetIsConnected = false;

#define ETHERNET_PIN 10

void setup() {
  Serial.begin(9600);
  NetworkBegin();

}


void loop(){ 
   OSCBundle bundleIN;
   int size;
 
   if( (size = udp.parsePacket())>0)
   {
     while(size--)
       bundleIN.fill(udp.read());

      if(!bundleIN.hasError())
        bundleIN.route("/tone", routeTone);
   }
}

char * numToOSCAddress( int pin){
    static char s[10];
    int i = 9;
  
    s[i--]= '\0';
  do
    {
    s[i] = "0123456789"[pin % 10];
                --i;
                pin /= 10;
    }
    while(pin && i);
    s[i] = '/';
    return &s[i];
}

void routeTone(OSCMessage &msg, int addrOffset ){
  //iterate through all the analog pins
  Serial.print("routeTone, ");
  
  float oscData;
  
  for(byte pin = 0; pin < NUM_DIGITAL_PINS; pin++){
    //match against the pin number strings
    int pinMatched = msg.match(numToOSCAddress(pin), addrOffset);
    if(pinMatched){
      unsigned int frequency = 0;
      //if it has an int, then it's an integers frequency in Hz
      if (msg.isInt(0)){        
        frequency = msg.getInt(0);
      } //otherwise it's a floating point frequency in Hz
      else if(msg.isFloat(0)){
        frequency = msg.getFloat(0);
      }
      else
        noTone(pin);
      if(frequency>0)
      {
         if(msg.isInt(1))
           //tone(pin, frequency, msg.getInt(1));
           oscData = frequency;
         else
           //tone(pin, frequency);
           oscData = frequency;
      }
      Serial.println(oscData);
    }
  }
}


void NetworkBegin(){
  while (millis() < 4000) {
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
