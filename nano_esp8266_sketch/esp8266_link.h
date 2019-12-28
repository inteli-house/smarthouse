/*
 * Copyright (c) 2019 Inteli-House.
 * See LICENSE for more information
 */
#ifndef _ESP8266_LINK_H
#define _ESP8266_LINK_H

#include <Arduino.h>

#include "constants.h"

extern bool wifiConnected;
extern bool serverConnected;
 
extern int mReceiveErrorCounter;
extern int mServerErrorCounter;
extern int mWifiErrorCounter;

void initializeESP_link() ;
bool startWifiConnection();
boolean connectWiFi(String _ssid, String _pwd);
void wifiInitATCommands();
void resetWifi ();
void closeConnection();

void resetWatchDog (void);
void errorHalt(String msg);

boolean requestSendPacket(bool firstStart,bool wasDisconnected);
void receiveRoutine();
void serverConnectRoutine();
bool udpServerConnect();
bool connectWifiRoutine(String _ssid, String _passw);
void logLine (char *line);
#endif // _ESP8266_LINK_H
