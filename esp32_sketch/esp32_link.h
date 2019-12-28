/*
 * Copyright (c) 2019 Inteli-House.
 * See LICENSE for more information
 */
#ifndef _ESP32_LINK_H
#define _ESP32_LINK_H

#include "constants.h"

extern bool wifiConnected;
extern bool serverConnected;

extern int mReceiveErrorCounter;
extern int mServerErrorCounter;
extern int mWifiErrorCounter;

void initializeESP_link();
bool startWifiConnection();
bool connectWiFi(char *_ssid, char *_pwd);

void errorHalt(char *msg);
void wifiDisconnect();
bool requestSendPacket(bool firstStart,bool wasDisconnected);
void receiveRoutine();
void serverConnectRoutine();
bool udpServerConnect();
bool connectWifiRoutine(char *_ssid, char *_passw);
void logLine (char *line);
#endif // _ESP32_LINK_H
