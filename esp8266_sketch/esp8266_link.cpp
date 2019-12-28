#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "esp8266_link.h"
#include "communication.h"
#include "sensors.h"
#include "cBase64.h"

bool wifiConnected = false;
bool serverConnected = false;

int mReceiveErrorCounter = 0;
int mServerErrorCounter = 0;
int mWifiErrorCounter = 0;

#define DEBUG_CONSOLE

IPAddress ServerIP;
WiFiUDP udp;

static char lenReserved[4];
static bool packetReceived = false;
static char result[REC_SIZE];

void initializeESP_link() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
}
bool requestSendPacket(bool firstStart, bool wasDisconnected) {
#ifdef DEBUG_CONSOLE
  logLine("request sent");
#endif;
  char *sendBuffEncoded = requestSend(firstStart, wasDisconnected);
  int encodedLen = base64_enc_len(SEND_SIZE);

  udp.beginPacket(DEST_IP, DEST_PORT);
  udp.write(sendBuffEncoded, encodedLen + 2);
  udp.endPacket();

  return true;
}
// Print error message and loop stop.
void errorHalt(char *msg) {
  while (true);
}
void processReceived (char *_res, int len ) {
  int decodedLen = base64_dec_len(_res, len);
  base64_decode(Constants::resultDecoded(), _res, len);
  char *resultDecoded = Constants::resultDecoded();
  if (crc8((uint8_t*)resultDecoded, decodedLen) == (uint8_t)resultDecoded[decodedLen]) {
    packetReceivedCRC_OK = true;
    mReceiveErrorCounter = 0;

    if (parseReceived(resultDecoded) != 0) {
      logLine("error halt");
      errorHalt("");
    }
  }
  else {
    packetReceivedCRC_OK = false;
  }
}
void receiveRoutine() {
  bool received = false;
  memset(result, 0, REC_SIZE);

  for (int i = 1; i <= 10; i++)
  {
    int cb = udp.parsePacket();
    if (cb) {
      for (int l = 0; l < cb ; l++) {
        udp.read(result + l, 1);
      }
      processReceived(result, cb);
      received = true;
#ifdef DEBUG_CONSOLE
      logLine("response received");
#endif;
      break;
    }
    else {
      delay(50);
    }
  }
  if (!received) {
    mReceiveErrorCounter++;
  }
}
void wifiDisconnect() {
  WiFi.disconnect();
}

bool connectWifiRoutine(char *_ssid, char *_passw) {
  for (int i = 1; i <= RECONNECTION_TRIES; i++) {
    if (connectWiFi(_ssid, _passw)) {
      wifiConnected = true;
      break;
    }
    if (i == RECONNECTION_TRIES) {
      wifiConnected = false;
    }
    delay(100);
  }
  return  wifiConnected;
}
bool connectWiFi(char *ssid, char *pwd) {
  WiFi.begin(ssid, pwd);   //Connect to access point
  int i = 0;
  while (WiFi.status() != WL_CONNECTED && i < CONNECTION_TRIES) {
    delay(500);
#ifdef DEBUG_CONSOLE
    Serial.print(".");
#endif
    i++;
  }
  if (WiFi.status() == WL_CONNECTED ) {
#ifdef DEBUG_CONSOLE
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif
    return true;
  }
  else {
    return false;
  }
}
bool startWifiConnection() {

  if (connectWifiRoutine( SSID , PASSW)) {
    delay(2000);  // Let the link stabilize.
    return true;
  }
  else {
    return false;
  }
}
void serverConnectRoutine() {
  serverConnected = udpServerConnect();

  if (!serverConnected) {
    mServerErrorCounter++;
  }
  else {
    mServerErrorCounter = 0;
    mReceiveErrorCounter = 0;
  }
}
bool establishUdpLink(char *ip, int port) {
  if (udp.begin(DEST_PORT)) {
    return true;
  }
  return false;
}
bool udpServerConnect () {
  bool udpEstablishSuccess = false;
  for (int i = 1; i <= CONNECTION_TRIES ; i++) {

    udpEstablishSuccess = establishUdpLink(DEST_IP, DEST_PORT);
    delay(500);
    if (udpEstablishSuccess) {
      break;
    }
    if (i == CONNECTION_TRIES) {
      udpEstablishSuccess = false;
    }
  }
  return udpEstablishSuccess;
}
void logLine (char *line) {
  Serial.println(line);
}
