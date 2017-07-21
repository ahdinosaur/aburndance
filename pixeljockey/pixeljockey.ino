/*
  ESP32 mDNS responder sample + OPC
  This is an example of an OPC server that is accessible
  via http://pj.local URL thanks to mDNS responder.
  Instructions:
  - Update WiFi SSID and password as necessary.
  - Flash the sketch to the ESP32 board
  - At startup the device will output it's IP and OPC Port to Serial.
  - Install host software:
    - For Linux, install Avahi (http://avahi.org/).
    - For Windows, install Bonjour (http://www.apple.com/support/bonjour/).
    - For Mac OSX and iOS support is built in through Bonjour already.
  - Point your browser to http://esp32.local, you should see a response.

  - Connect your OPC Client and send data. Received headers will be output to Serial.
*/
//==============================================================================
#include <WiFi.h>
#include "OpcServer.h"
#include <ESPmDNS.h>

//==============================================================================
// Configuration
//==============================================================================
const char* WIFI_SSID = "<WIFI_SSID>";
const char* WIFI_PASS = "<WIFI_PASS>";

const int OPC_PORT = 7890;
const int OPC_CHANNEL = 1;      // Channel to respond to in addition to 0 (broadcast channel)
const int OPC_MAX_CLIENTS = 2;  // Maxiumum Number of Simultaneous OPC Clients

// Size of OPC Read Buffer
// * Should probably size to number of pixels * 3 plus 4 byte header
// * If an OPC Message is received that is longer, the remaining pixels are discarded
// * If you are receiving OPC Messages that are longer than the number of pixels you may see increased
//   performance by increasing the buffer size to hold the entire message
const int OPC_MAX_PIXELS = 64 * 64 * 2;
const int OPC_BUFFER_SIZE = OPC_MAX_PIXELS * 3 + OPC_HEADER_BYTES;
//------------------------------------------------------------------------------

// Callback when a full OPC Message has been received
void cbOpcMessage(uint8_t channel, uint8_t command, uint16_t length, uint8_t* data) {
  Serial.print("chn:");
  Serial.print(channel);
  Serial.print("cmd:");
  Serial.print(command);
  Serial.print("len:");
  Serial.println(length);
}

// Callback when a client is connected
void cbOpcClientConnected(WiFiClient& client) {
  Serial.print("New OPC Client: ");
  Serial.println(client.remoteIP());
}

// Callback when a client is disconnected
void cbOpcClientDisconnected(OpcClient& opcClient) {
  Serial.print("Client Disconnected: ");
  Serial.println(opcClient.ipAddress);
}

//==============================================================================
// OpcServer Init
//==============================================================================
WiFiServer server = WiFiServer(OPC_PORT);
OpcClient opcClients[OPC_MAX_CLIENTS];
uint8_t buffer[OPC_BUFFER_SIZE * OPC_MAX_CLIENTS];
OpcServer opcServer = OpcServer(server, OPC_CHANNEL, opcClients, OPC_MAX_CLIENTS, buffer, OPC_BUFFER_SIZE);
//------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  if (WiFi.SSID() != WIFI_SSID) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  opcServer.setMsgReceivedCallback(cbOpcMessage);
  opcServer.setClientConnectedCallback(cbOpcClientConnected);
  opcServer.setClientDisconnectedCallback(cbOpcClientDisconnected);

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "pj.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("pj")) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  // Start TCP (OPC) server
  opcServer.begin();

  Serial.print("\nOPC Server: ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(OPC_PORT);

  // Add service to MDNS-SD
  MDNS.addService("_opc", "_tcp", OPC_PORT);
}

void loop() { opcServer.process(); }
