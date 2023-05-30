#include <WiFi.h>
#include <WiFiUdp.h>
//#include <ArduinoOSC.h>
#include <MicroOsc.h>
#include <MicroOscSlip.h>

/************************WIFI CONTROL STUFF***************************/

const char * networkName = "TerMirinator: Genysis"; // name of network
const char * networkPswd = "ThisSucks"; // network password

const char * udpAddress = "10.0.10.0"; // ESP32 naming scheme: 10.0.10.x
const int udpPort = 3333;

bool connected = false;

WiFiUDP udp;
/*********************************************************************/


/************************LED CONTROL STUFF****************************/
const int LED_PIN = 3;
/*********************************************************************/


/************************OSC CONTROL STUFF****************************/
MicroOscSlip<64> myMicroOsc(&Serial);
/*********************************************************************/


/************************OSC CONTROL STUFF****************************/
int angleX = 0;
int angleY = 0;
int angleZ = 0;
/*********************************************************************/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //why 9600? because I said so damn it
  WiFiScan(); //see function notes

  ConnectToWiFi(networkName, networkPswd);
}

void loop() {
  // put your main code here, to run repeatedly:
  // if (connected) {
  //   Serial.println("WiFi is Connected!");
  // } else {
  //   Serial.println("No WiFi connection");
  // }

  delay(5000);

}

void ConnectToWiFi(const char * ssid, const char * pwd){
  /* The Wifi Connection function. For initial setup, cycle the Arduino once,
  to make sure the network you want to connect to is available, then fill in
  the network name and password variables at the top (if not done already) */

  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  
  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

void WiFiEvent(WiFiEvent_t event){
  /* This function handles wifi events. At Runtime, feel free to disable
  the repeating print function just to save some computing power. */
    switch(event) {
      case ARDUINO_EVENT_WIFI_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void WiFiScan(){
  /* This function is run at start to check for available WiFi connections.
  If you do not see the network you want to connect to in the list, check
  the router on another device to make sure it is active and running. */
  Serial.println("Scan start");

  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
      Serial.print(n);
      Serial.println(" networks found");
      Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
      for (int i = 0; i < n; ++i) {
          // Print SSID and RSSI for each network found
          Serial.printf("%2d",i + 1);
          Serial.print(" | ");
          Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
          Serial.print(" | ");
          Serial.printf("%4d", WiFi.RSSI(i));
          Serial.print(" | ");
          Serial.printf("%2d", WiFi.channel(i));
          Serial.print(" | ");
          switch (WiFi.encryptionType(i))
          {
          case WIFI_AUTH_OPEN:
              Serial.print("open");
              break;
          case WIFI_AUTH_WEP:
              Serial.print("WEP");
              break;
          case WIFI_AUTH_WPA_PSK:
              Serial.print("WPA");
              break;
          case WIFI_AUTH_WPA2_PSK:
              Serial.print("WPA2");
              break;
          case WIFI_AUTH_WPA_WPA2_PSK:
              Serial.print("WPA+WPA2");
              break;
          case WIFI_AUTH_WPA2_ENTERPRISE:
              Serial.print("WPA2-EAP");
              break;
          case WIFI_AUTH_WPA3_PSK:
              Serial.print("WPA3");
              break;
          case WIFI_AUTH_WPA2_WPA3_PSK:
              Serial.print("WPA2+WPA3");
              break;
          case WIFI_AUTH_WAPI_PSK:
              Serial.print("WAPI");
              break;
          default:
              Serial.print("unknown");
          }
          Serial.println();
          delay(10);
      }
  }
  Serial.println("");

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
}
