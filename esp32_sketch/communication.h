/*
 * Copyright (c) 2019 Inteli-House.
 * See LICENSE for more information
 */
#ifndef _COMMUNICATION_H
#define _COMMUNICATION_H

extern int histeresis;
extern int lastHisteresis;
extern int setPoint;
extern int lastSetPoint;
extern int tempOffset;
extern int lastTempOffset;
extern bool heatEnabled;
extern bool tempInvert;
extern int modeAgregate;
extern int lastModeAgregate;
extern bool heatOn;
extern bool lastheatOn;
extern int temperature;
extern bool out1, out2, out3, out4, out5, out6, out7, out8;
extern int analog1;
extern int analog2;
extern int analog3;
extern int analog4;
extern bool digital1, digital2, digital3, digital4, digital5, digital6, digital7, digital8;

int parseReceived (char *);
char * requestSend(bool firstStart,bool wasDisconnected);

#endif // COMMUNICATION_H
