#include "esp8266_link.h"
#include "communication.h"
#include "sensors.h"
#include "cBase64.h"
#include <SoftwareSerial.h>

static bool wifiConnected = false;
static bool serverConnected = false;

static int mReceiveErrorCounter = 0;
static int mServerErrorCounter = 0;
static int mWifiErrorCounter = 0;

unsigned long deadline;  // Global time deadline for receiving characters.
static char  recTemp[10];
static char comp_buffer[CB_SIZE + 1] = {0,}; // Fill buffer with string terminator.

static char lenReserved[4];
static bool packetReceived = false;
static char result[REC_SIZE];


#define esp Serial    // use Serial for ESP
SoftwareSerial dbg(8, 9); // use pins 8, 9 for software serial

int getPacketLength();
boolean echoFind(String keyword, boolean do_echo, unsigned long  intTimeout = TIMEOUT);
boolean echoCommand(String cmd, String ack, boolean halt_on_fail, unsigned long  intT, bool echo = false);

void initializeESP_link() {
  esp.begin(9600);  //ESP8266 baudRate
  dbg.begin(38400); //debug line baudRate
}

boolean requestSendPacket(bool firstStart, bool wasDisconnected) {
  char *sendBuffEncoded = requestSend(firstStart, wasDisconnected);
  int encodedLen = base64_enc_len(SEND_SIZE);
  int len = encodedLen + 4;

  if (!echoCommand(Constants::CSTR_AT_CIPSEND() + len, ">", CONTINUE, TIMEOUT, NO_ECHO))
  {
    echoCommand(Constants::CSTR_AT_CIPCLOSE(), "", CONTINUE, TIMEOUT, NO_ECHO);
    errorHalt("Timeout");
  }
  return echoCommand(sendBuffEncoded, Constants::CSTR_AT_SENDOK(), CONTINUE, TIMEOUT, DO_ECHO);
}

void enableTimerInterrupt () {
  TCNT1 = PRELOAD_TIMER;
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
}
void disableTimerInterrupt() {
  TIMSK1 &= ~(1 << TOIE1);
}
// Print error message and loop stop.
void errorHalt(String msg){
#ifdef DEBUG_CONSOLE
  dbg.println(msg);
  dbg.println("HALT");
#endif
  disableTimerInterrupt();
  while (true);
}

char getChar(bool checkDeadline = true){

  while (!esp.available()) {
    if (checkDeadline)
    {
      if (millis() >= deadline) {
        return 0;
      }
    }
    resetWatchDog();
  }
  return esp.read();
}
void processReceived (char *_res, int len ) {
  int decodedLen = base64_dec_len(_res, len);
  base64_decode(Constants::resultDecoded(), _res, len);
  char *resultDecoded = Constants::resultDecoded();
  if (crc8((uint8_t*)resultDecoded, decodedLen) == (uint8_t)resultDecoded[decodedLen]) {
    packetReceivedCRC_OK = true;
    mReceiveErrorCounter = 0;

    if (parseReceived(resultDecoded) != 0) {
      errorHalt("");
    }
  }
  else {
    packetReceivedCRC_OK = false;
  }
}

void processReceivedPacket () {
  int i =  getPacketLength();
  memset(result, 0, REC_SIZE);
  deadline = millis() + TIMEOUT;
  for (int l = 0; l < i ; l++) {
    result[l] = getChar();
  }
  processReceived(result, i);
}

void receiveRoutine(){
  if (echoFind(Constants::cmpIPD(), NO_ECHO, 1600))
  {
    processReceivedPacket();
  }
  else {
#ifdef DEBUG_CONSOLE
    dbg.println("\r\nNO CONN");
#endif
    mReceiveErrorCounter++;
  }
}

bool connectWifiRoutine(String _ssid, String _passw) {
  for (int i = 1; i <= CONNECTION_TRIES; i++)
  {
    if (connectWiFi(_ssid, _passw)) {
      wifiConnected = true;
      break;
    }

    if (i == CONNECTION_TRIES) {
      wifiConnected = false;
    }
    resetWatchDog();
    delay(100);
  }
  return  wifiConnected;
}
boolean connectWiFi(String _ssid, String _pwd){
  String cmd = Constants::CSTR_AT_CWJAP() + _ssid + "\",\"" + _pwd + "\"";
  if (echoCommand(cmd, Constants::CSTR_AT_OK(), CONTINUE, 2 * TIMEOUT , DO_ECHO)) // Join Access Point
  {
#ifdef DEBUG_CONSOLE
    dbg.println("\nConnected to WiFi.");
#endif
    return true;
  }
  else
  {
#ifdef DEBUG_CONSOLE
    dbg.println("\nConnection to WiFi failed.");
#endif
    return false;
  }
}
bool startWifiConnection() {
  wifiInitATCommands();
  delay(200);  // Let things settle down for a bit...
  resetWatchDog();

  if (connectWifiRoutine( SSID , PASSW)) {
    enableTimerInterrupt();
    delay(2000);  // Let the link stabilize.
    disableTimerInterrupt();
    return true;
  }
  else {
    return false;
  }
}

void wifiInitATCommands() {
  echoCommand(Constants::CSTR_AT_RST(), Constants::CSTR_READY(), CONTINUE, TIMEOUT, DO_ECHO);    // Reset the module.
  enableTimerInterrupt();
  delay(2500);
  disableTimerInterrupt();
  echoCommand(Constants::CSTR_AT_AUTOCN(), Constants::CSTR_AT_OK(), CONTINUE, TIMEOUT, DO_ECHO);
  echoCommand(Constants::CSTR_AT_CWMODE(), Constants::CSTR_AT_OK(), CONTINUE, TIMEOUT, DO_ECHO); // Set module into station mode.
  echoCommand(Constants::CSTR_AT_CIPMUX(), Constants::CSTR_AT_OK(), CONTINUE, TIMEOUT, DO_ECHO); // Allow multiple connections. Necessary for TCP link.
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

boolean establishUdpLink(String ip, int port){
  String cmd = Constants::CSTR_AT_CIPSTART() + ip + "\"," + port;
  return echoCommand(cmd, Constants::CSTR_AT_OK(), CONTINUE, TIMEOUT, DO_ECHO);
}
bool udpServerConnect () {
  bool sConnected = false;
  bool udpEstablishSuccess = false;
  for (int i = 1; i <= CONNECTION_TRIES ; i++) {

    udpEstablishSuccess = establishUdpLink(DEST_IP, DEST_PORT);
    enableTimerInterrupt();
    delay(500);  // Once again, let the link stabilize.
    disableTimerInterrupt();
    if (udpEstablishSuccess) {
      break;
    }
    if (i == CONNECTION_TRIES) {
      sConnected = false;
      udpEstablishSuccess = false;
    }
  }
  if (udpEstablishSuccess) {
    for (int j = 1; j <= CONNECTION_TRIES ; j++) {
      bool success = echoCommand(Constants::CSTR_AT_CIPSTATUS(), Constants::CSTR_AT_OK(), CONTINUE, TIMEOUT, DO_ECHO);
      enableTimerInterrupt();
      delay(500);
      disableTimerInterrupt();
      if (success ) {
        sConnected = true;
        break;
      }
      if (j == CONNECTION_TRIES) {
        //errorHalt("Connection failed");
        sConnected = false;
        break;
      }
    }
  }
  return sConnected;
}

void closeConnection() {
  echoCommand(Constants::CSTR_AT_CIPCLOSE(), "", CONTINUE, TIMEOUT, NO_ECHO);
}

void resetWifi () {
  digitalWrite(RST_WIFI, false);
  enableTimerInterrupt();
  delay(500);
  digitalWrite(RST_WIFI, true);
  delay(500);
  disableTimerInterrupt();
}


// Get char from ESP8266 and also shift it into the comparison buffer.
char getCharAndBuffer(bool checkDeadline = true ){
  char b = getChar(checkDeadline);
  char *cb_src, *cb_dst;
  cb_src = comp_buffer + 1;
  cb_dst = comp_buffer;
  for (int i = 1; i < (CB_SIZE - 1); i++) {
    *cb_dst++ = *cb_src++;
  }
  *cb_dst = b;
  return b;
}

boolean cb_match(String &s){
  return strcmp(s.c_str(), comp_buffer + CB_SIZE - 1 - s.length()) == 0; // Return true on match.
}

// Read characters from ESP8266 module and echo to serial until keyword occurs or timeout.
boolean echoFind(String keyword, boolean do_echo, unsigned long  intTimeout = TIMEOUT) {
  bool found = false;
  deadline = millis() + intTimeout;
  while ( millis() < deadline)
  {

    char ch = getCharAndBuffer(true);
    if (do_echo) {
      dbg.write(ch);
    }
    if (cb_match(keyword)) {
      found =  true;
      break;
    }
  }
  return found;  // Timed out
}
void serialFlush() {
  while (esp.available() > 0) {
    esp.read();
  }
}
// Echo module output until 3 newlines encountered.
// (Used when we're indifferent to "OK" vs. "no change" responses.)
void echoSkip()
{
  echoFind("\n", NO_ECHO, 200);       // Search for nl at end of command echo
  echoFind("\n", NO_ECHO, 200);       // Search for 2nd nl at end of response.
  echoFind("\n", NO_ECHO, 200);       // Search for 3rd nl at end of blank line.
}

// Send a command to the module and wait for acknowledgement string
// (or flush module output if no ack specified).
// Echoes all data received to the serial monitor.

boolean echoCommand(String cmd, String ack, boolean halt_on_fail, unsigned long  intT, boolean echo = false)
{
  // Send the command to the ESP8266.
  serialFlush();
  esp.print(cmd);
  esp.write("\015\012"); // Append carriage-return+linefeed to commands.
  esp.flush();
  resetWatchDog();

  // If no ack response specified, skip all available module output.
  if (strlen(ack.c_str()) == 0) {
    echoSkip();
  }
  else
  {
    // Otherwise wait for ack.
    if (!echoFind(ack, echo, intT))   // timed out waiting for ack string
    {
      if (halt_on_fail)
        errorHalt(cmd + " failed");// Critical failure halt.
      else
        return false;            // Let the caller handle it.
    }
  }
  return true;                   // ack blank or ack found
}

// Data packets have the format "+IPD,0,1024:something....". This routine gets
// the second number preceding the ":" that indicates the number of characters
// in the packet.
int getPacketLength(){
  deadline = millis() + TIMEOUT;

  while (getChar(true) != ',');
  for (int i = 0; i < 4; i++)
  {
    char c = getChar(true);

    if (c == ':')
    {
      lenReserved[i] = 0; // Terminate string.
      break;
    }
    lenReserved[i] = c;
  }
  lenReserved[3] = 0;
  return atoi(lenReserved);
}

void resetWatchDog (void) {
  digitalWrite(RST_WATCHDOG, digitalRead(RST_WATCHDOG) ^ 1);
}
void logLine (char *line) {
  dbg.print(line);
  dbg.println("");
}
