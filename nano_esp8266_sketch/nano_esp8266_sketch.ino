/*
  Copyright (c) 2019 Inteli-House.
  See LICENSE for more information
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <EepromUtil.h>

#include "esp8266_link.h"
#include "sensors.h"
#include "constants.h"
#include "communication.h"

static int initialized_device = 0;
static  int initialized_wifi  = 0;

void resetEEprom();


ISR(TIMER1_OVF_vect)        // interrupt service routine for watchdog reset
{
  TCNT1 = PRELOAD_TIMER;            // preload timer
  digitalWrite(RST_WATCHDOG, digitalRead(RST_WATCHDOG) ^ 1);
}
void setup()
{
  pinMode(RST_WATCHDOG, OUTPUT);

  delay(50);
  noInterrupts();           // disable all interrupts

  TCCR1A = 0;
  TCCR1B = 0;

  TCNT1 = PRELOAD_TIMER;       // preload timer
  TCCR1B |= (1 << CS12);       // prescaler
  TIMSK1 &= ~(1 << OCIE1A);
  interrupts();             // enable all interrupts
  resetWatchDog();
  
  pinMode(INPUT_MAPPING_ONE_WIRE_BUS, INPUT);
  pinMode(OUTPUT_MAPPING_OUT1, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT2, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT3, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT4, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT5, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT6, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT7, OUTPUT);
  pinMode(OUTPUT_MAPPING_OUT8, OUTPUT);

  pinMode(RST_WIFI, OUTPUT);
  resetWifi ();

#ifdef DEBUG_CONSOLE
  logLine("start");
#endif

  initializeSensors();
  initializeESP_link();


  EepromUtil::eeprom_read_int(EEPROM_DEVICE_INIT_ADDR, &initialized_device);
  if (initialized_device == EEPROM_INITIALIZED_VALUE ) {
#ifdef DEBUG_CONSOLE
    logLine("mem ok");
#endif
    EepromUtil::eeprom_read_int(EEPROM_MODE_VALUE_ADDR, &modeAgregate);
    EepromUtil::eeprom_read_int(EEPROM_TEMP_SETPOINT_ADDR, &setPoint);
    EepromUtil::eeprom_read_int(EEPROM_HISTERESIS_VALUE_ADDR, &histeresis);
    EepromUtil::eeprom_read_int(EEPROM_TEMP_OFFSET_VALUE_ADDR, &tempOffset);

    heatEnabled = modeAgregate & 0x01;
    tempInvert = (modeAgregate >> 1) & 0x01;

    lastModeAgregate = modeAgregate;
    lastSetPoint = setPoint;
    lastHisteresis = histeresis;
    lastTempOffset = tempOffset;


    EepromUtil::eeprom_read_int(EEPROM_WIFI_INIT_ADDR, &initialized_wifi);
    if (initialized_wifi == EEPROM_WIFI_INIT_VALUE ) {
      mServerErrorCounter = 0;
      logLine("start wifi");
      startWifiConnection();
    }
    else {
      mServerErrorCounter = 0;
      if (startWifiConnection()) {
        EepromUtil::eeprom_write_int(EEPROM_WIFI_INIT_ADDR, EEPROM_WIFI_INIT_VALUE);
        initialized_wifi = EEPROM_WIFI_INIT_VALUE;
      }
    }
  }
  else {
    resetEEprom();
    mServerErrorCounter = 0;
    EepromUtil::eeprom_write_int(EEPROM_DEVICE_INIT_ADDR, EEPROM_INITIALIZED_VALUE);

    if (startWifiConnection()) {
      EepromUtil::eeprom_write_int(EEPROM_WIFI_INIT_ADDR, EEPROM_WIFI_INIT_VALUE);
      initialized_wifi = EEPROM_WIFI_INIT_VALUE;
    }
  }
  delay(100);
}

void loop()
{
  bool firstStart = true;
  bool wasDisconected = true;
  int requestTempCount = 0;


  while (true) {
    if (wifiConnected && serverConnected) {
      resetWatchDog();

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
      resetWatchDog();
      boolean sucess = requestSendPacket(false, false);
      resetWatchDog();
      delay(10);
      receiveRoutine();

      if (packetReceivedCRC_OK) {
        packetReceivedCRC_OK = false;

        if (modeAgregate != lastModeAgregate) {

          EepromUtil::eeprom_write_int(EEPROM_MODE_VALUE_ADDR, modeAgregate);
          lastModeAgregate = modeAgregate;
          temperatureRegulation();
        }
        if (setPoint != lastSetPoint) {

          EepromUtil::eeprom_write_int(EEPROM_TEMP_SETPOINT_ADDR, setPoint);
          lastSetPoint = setPoint;
          temperatureRegulation();
        }
        if (histeresis != lastHisteresis) {

          EepromUtil::eeprom_write_int(EEPROM_HISTERESIS_VALUE_ADDR, histeresis);
          lastHisteresis = histeresis;
        }
        if (tempOffset != lastTempOffset) {

          EepromUtil::eeprom_write_int(EEPROM_TEMP_OFFSET_VALUE_ADDR, tempOffset);
          lastTempOffset = tempOffset;
        }
      }

      if (mReceiveErrorCounter > GLOBAL_REPEAT_ERROR_COUNT) {  

        if (mReceiveErrorCounter > 3 * GLOBAL_REPEAT_ERROR_COUNT) { 
          wifiConnected = false;
        }
        else {
          closeConnection() ;
          serverConnectRoutine();
        }
        wasDisconected = true;
      }
    }
    else {
      if (!wifiConnected  && initialized_wifi == EEPROM_WIFI_INIT_VALUE )  {
#ifdef DEBUG_CONSOLE
        logLine("reseting wifi");
#endif
        resetWifi ();
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
        serverConnectRoutine();
        if (mServerErrorCounter  > GLOBAL_SERVER_REPEAT_ERROR_COUNT) { 
          wifiConnected = false;
        }
      }
      else {
        errorHalt("RESET");
      }
    }
    delay(100);
  }
  errorHalt("RESET");
}

void temperatureRegulation() {
  if (heatEnabled) {
    if (temperature > setPoint ) {
      digitalWrite(OUTPUT_MAPPING_OUT1, tempInvert ? true : false);
      heatOn = tempInvert ? true : false;
      if (heatOn != lastheatOn) {
        EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR1, tempInvert ? 1 : 0);
        lastheatOn = heatOn;
      }
    }
    else if  (temperature < (setPoint - histeresis)) {
      digitalWrite(OUTPUT_MAPPING_OUT1, tempInvert ? false : true);
      heatOn = tempInvert ? false : true;
      if (heatOn != lastheatOn) {
        EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR1, tempInvert ? 0 : 1);
        lastheatOn = heatOn;
      }
    }
  }
}
void resetEEprom () {
#ifdef DEBUG_CONSOLE
  logLine("erasing");
#endif
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR1, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR2, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR3, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR4, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR5, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR6, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR7, 0);
  EepromUtil::eeprom_write_int(EEPROM_RELAY_INIT_ADDR8, 0);

  EepromUtil::eeprom_write_int(EEPROM_MODE_VALUE_ADDR, 0);
  EepromUtil::eeprom_write_int( EEPROM_TEMP_SETPOINT_ADDR, 0);
  EepromUtil::eeprom_write_int(EEPROM_HISTERESIS_VALUE_ADDR, 5);
  EepromUtil::eeprom_write_int(EEPROM_TEMP_OFFSET_VALUE_ADDR, 0);
}
