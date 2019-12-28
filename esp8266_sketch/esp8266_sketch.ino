/*
  Copyright (c) 2019 Inteli-House.
  See LICENSE for more information
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EEPROM.h>

#include "esp8266_link.h"
#include "sensors.h"
#include "constants.h"
#include "communication.h"


static int initialized_device = 0;
static int initialized_wifi  = 0;

void resetEEprom();


void setup()
{

  delay(50);
  noInterrupts();           // disable all interrupts

  EEPROM.begin(512);
  delay(100);
  pinMode(INPUT_MAPPING_ONE_WIRE_BUS, INPUT);
  pinMode(OUTPUT_MAPPING_OUT1, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT2, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT3, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT4, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT5, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT6, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT7, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT8, OUTPUT);

#ifdef DEBUG_CONSOLE
  logLine("start");
#endif
  initializeESP_link();
  initializeSensors();
  
  initialized_device = eePromGetInt(EEPROM_DEVICE_INIT_ADDR);
  if (initialized_device == EEPROM_INITIALIZED_VALUE ) {
#ifdef DEBUG_CONSOLE
    logLine("mem ok");
#endif
    modeAgregate = eePromGetInt(EEPROM_MODE_VALUE_ADDR);
    setPoint = eePromGetInt(EEPROM_TEMP_SETPOINT_ADDR);
    histeresis = eePromGetInt(EEPROM_HISTERESIS_VALUE_ADDR);
    tempOffset = eePromGetInt(EEPROM_TEMP_OFFSET_VALUE_ADDR);

    heatEnabled = modeAgregate & 0x01;
    tempInvert = (modeAgregate >> 1) & 0x01;

    lastModeAgregate = modeAgregate;
    lastSetPoint = setPoint;
    lastHisteresis = histeresis;
    lastTempOffset = tempOffset;


    initialized_wifi = eePromGetInt(EEPROM_WIFI_INIT_ADDR);
    if (initialized_wifi == EEPROM_WIFI_INIT_VALUE ) {
      mServerErrorCounter = 0;
      startWifiConnection();
    }
    else {
      mServerErrorCounter = 0;
      if (startWifiConnection()) {
        eePromWriteInt(EEPROM_WIFI_INIT_ADDR, EEPROM_WIFI_INIT_VALUE);
        initialized_wifi = EEPROM_WIFI_INIT_VALUE;
      }
    }
  }
  else {
    resetEEprom();
    mServerErrorCounter = 0;
    eePromWriteInt(EEPROM_DEVICE_INIT_ADDR, EEPROM_INITIALIZED_VALUE);
    if (startWifiConnection()) {
      eePromWriteInt(EEPROM_WIFI_INIT_ADDR, EEPROM_WIFI_INIT_VALUE);
      initialized_wifi = EEPROM_WIFI_INIT_VALUE;
    }
  }
  delay(100);
}

void loop()
{
  bool firstStart = true;
  int requestTempCount = 0;


  while (true) {
    if (wifiConnected && serverConnected) {
      if (requestTempCount % 50 == 0 || requestTempCount == 1) {
        requestTempCount = 1;

        float tempT = getTemperature();

        if (tempT != -127)
        {
          temperature = ((int) 100 * tempT / 10) + tempOffset;
          temperatureRegulation();
        }
      }
      requestTempCount++;
      boolean sucess = requestSendPacket(false, false);
      delay(10);
      receiveRoutine();

      if (packetReceivedCRC_OK) {
        packetReceivedCRC_OK = false;

        if (modeAgregate != lastModeAgregate) {

          eePromWriteInt(EEPROM_MODE_VALUE_ADDR, modeAgregate);
          lastModeAgregate = modeAgregate;
          temperatureRegulation();
        }
        if (setPoint != lastSetPoint) {

          eePromWriteInt(EEPROM_TEMP_SETPOINT_ADDR, setPoint);
          lastSetPoint = setPoint;
          temperatureRegulation();
        }
        if (histeresis != lastHisteresis) {

          eePromWriteInt(EEPROM_HISTERESIS_VALUE_ADDR, histeresis);
          lastHisteresis = histeresis;
        }
        if (tempOffset != lastTempOffset) {

          eePromWriteInt(EEPROM_TEMP_OFFSET_VALUE_ADDR, tempOffset);
          lastTempOffset = tempOffset;
        }
      }

      if (mReceiveErrorCounter > 3 * GLOBAL_REPEAT_ERROR_COUNT) {
        wifiConnected = false;
      }
    }
    else {
      if (!wifiConnected  && initialized_wifi == EEPROM_WIFI_INIT_VALUE )  {
        wifiDisconnect();

#ifdef DEBUG_CONSOLE
        logLine("reseting wifi");
#endif

        mServerErrorCounter = 0;
        startWifiConnection();
        if (wifiConnected) {
          mReceiveErrorCounter = 0;
          serverConnectRoutine();
        }
        else {
          mWifiErrorCounter++;
          if (mWifiErrorCounter  > GLOBAL_WIFI_REPEAT_ERROR_COUNT) {
            errorHalt("");
          }
        }
      }
      else if (wifiConnected && !serverConnected  ) {
#ifdef DEBUG_CONSOLE
        logLine("serverConnectRoutine");
#endif
        serverConnectRoutine();
        if (mServerErrorCounter  > GLOBAL_SERVER_REPEAT_ERROR_COUNT) {
          wifiConnected = false;
        }
      }
      else {
        errorHalt("RESET");
      }
    }
    delay(1000);
  }
  errorHalt("RESET");
}

void temperatureRegulation() {
  if (heatEnabled) {
    if (temperature > setPoint ) {
      digitalWrite(OUTPUT_MAPPING_OUT1, tempInvert ? true : false);
      heatOn = tempInvert ? true : false;
      if (heatOn != lastheatOn) {
        eePromWriteInt(EEPROM_RELAY_INIT_ADDR1, tempInvert ? 1 : 0);
        lastheatOn = heatOn;
      }
    }
    else if  (temperature < (setPoint - histeresis)) {
      digitalWrite(OUTPUT_MAPPING_OUT1, tempInvert ? false : true);
      heatOn = tempInvert ? false : true;
      if (heatOn != lastheatOn) {
        eePromWriteInt(EEPROM_RELAY_INIT_ADDR1, tempInvert ? 0 : 1);
        lastheatOn = heatOn;
      }
    }
  }
}


void resetEEprom () {
#ifdef DEBUG_CONSOLE
  logLine("erasing");
#endif
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR1, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR2, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR3, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR4, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR5, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR6, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR7, 0);
  eePromWriteInt(EEPROM_RELAY_INIT_ADDR8, 0);

  eePromWriteInt(EEPROM_MODE_VALUE_ADDR, 0);
  eePromWriteInt(EEPROM_TEMP_SETPOINT_ADDR, 0);
  eePromWriteInt(EEPROM_HISTERESIS_VALUE_ADDR, 5);
  eePromWriteInt(EEPROM_TEMP_OFFSET_VALUE_ADDR, 0);
}

void eePromWriteInt(int pos, int val) {
  byte* p = (byte*) &val;
  EEPROM.write(pos, val);
  EEPROM.write(pos + 1, *(p + 1));
  EEPROM.write(pos + 2, *(p + 2));
  EEPROM.write(pos + 3, *(p + 3));
  delay(100);
  EEPROM.commit();
}
int eePromGetInt(int pos) {
  int val;
  byte* p = (byte*) &val;
  *p        = EEPROM.read(pos);
  *(p + 1)  = EEPROM.read(pos + 1);
  *(p + 2)  = EEPROM.read(pos + 2);
  *(p + 3)  = EEPROM.read(pos + 3);
  return val;
}
