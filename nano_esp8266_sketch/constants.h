/*
 Copyright (c) 2019 Inteli-House.
 See LICENSE for more information
 */
#include <Arduino.h>

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define DEBUG_CONSOLE

/* CONNECTION PARAMETERS -BEGIN*/
#define SSID                "WIFI SSID"             // change to your WIFI network name
#define PASSW               "WIFI PASSWORD"         // change to your WIFI password
#define DEST_IP             "SERVER DOMAIN NAME"    // change to server domain name (obtained via mobile app)
#define DEST_PORT           27735
#define DEVICE_ADDRESS      0x01
static const char registrationID[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //change to your unique registration ID (obtained via mobile app)
/* CONNECTION PARAMETERS -END*/

#define TIMEOUT     5000  // mS
#define EEPROM_INITIALIZED_VALUE 18345
#define EEPROM_WIFI_INIT_VALUE 10392
#define EEPROM_DEVICE_INIT_ADDR 0
#define EEPROM_WIFI_INIT_ADDR 4
#define EEPROM_SN_LENGTH 10 + 1
#define EEPROM_RELAY_INIT_ADDR1 100
#define EEPROM_RELAY_INIT_ADDR2 102
#define EEPROM_RELAY_INIT_ADDR3 104
#define EEPROM_RELAY_INIT_ADDR4 106
#define EEPROM_RELAY_INIT_ADDR5 108
#define EEPROM_RELAY_INIT_ADDR6 110
#define EEPROM_RELAY_INIT_ADDR7 112
#define EEPROM_RELAY_INIT_ADDR8 114

#define EEPROM_MODE_VALUE_ADDR 116
#define EEPROM_TEMP_SETPOINT_ADDR 118
#define EEPROM_HISTERESIS_VALUE_ADDR 120
#define EEPROM_TEMP_OFFSET_VALUE_ADDR 122

#define REC_SIZE  20
#define SEND_SIZE 24
#define CB_SIZE 20

#define PRELOAD_TIMER 40000

#define RST_WATCHDOG 11
#define RST_WIFI PD3
#define INPUT_MAPPING_ONE_WIRE_BUS PD4
#define OUTPUT_MAPPING_OUT1 A5 
#define OUTPUT_MAPPING_OUT2 A4 
#define OUTPUT_MAPPING_OUT3 A3 
#define OUTPUT_MAPPING_OUT4 A2 
#define OUTPUT_MAPPING_OUT5 A1 
#define OUTPUT_MAPPING_OUT6 A0 
#define OUTPUT_MAPPING_OUT7 12
#define OUTPUT_MAPPING_OUT8 13

#define CONTINUE false
#define CONNECTION_TRIES    4

#ifdef DEBUG_CONSOLE
#define DO_ECHO             true
#else
#define DO_ECHO             false
#endif
#define NO_ECHO             false

#define FUNC_STREAM 0x01 //basic streaming function
#define DEBUG_CONSOLE

static int GLOBAL_REPEAT_ERROR_COUNT = 10;
static int GLOBAL_WIFI_REPEAT_ERROR_COUNT = 2;
static int GLOBAL_SERVER_REPEAT_ERROR_COUNT = 2;

static bool packetReceivedCRC_OK = false;

class Constants {
  
public:
   

    static const char PROT_VER(void) // protocol version
    {
        static const char ch = {0x11};
        
        return ch;
    }
    static const char *startSeq(void) //start of sequence = 2 bytes
    {
        static const char ch[2] = {0x55, 0x55};
        
        return ch;
    }
    static char *resultDecoded(void) //result from server
    {
        static char ch[REC_SIZE];
        
        return ch;
    }
    static const char NullChar(void)
    {
        return '\0';
        
    } 
    
    static const String cmpIPD(void)
    {
        return "+IPD";
        
    }
    
   /*ESP 8266 COMMAND SET */
   
    static const String CSTR_AT(void)
    {  
           return "AT+";
    }
    static const String CSTR_AT_CIP(void)
    {
        return "CIP";
    }
    static const String CSTR_AT_CW(void)
    {
        return "CW";
        
    }
    static const String CSTR_AT_CWJAP(void)
    {
        return CSTR_AT() + CSTR_AT_CW() + "JAP=\"";
        
    }
    static const String CSTR_AT_CIPSTART(void)
    {
        return CSTR_AT() + CSTR_AT_CIP() + "START=\"UDP\",\"";
        
    }
    static const String CSTR_READY(void)
    {
        return "ready";
        
    }
    static const String CSTR_AT_RST(void)
    {
        return CSTR_AT() + "RST";
        
    }
    static const String CSTR_AT_GMR(void)
    {
        return CSTR_AT() + "GMR";
        
    }
    static const String CSTR_AT_CWMODE(void)
    {
        return CSTR_AT() + CSTR_AT_CW() + "MODE=1";
        
    }
    static const String CSTR_AT_CIPMUX(void)
    {
        return CSTR_AT() + CSTR_AT_CIP()  + "MUX=0";
        
    }
    static const String CSTR_AT_AUTOCN(void)
    {
        return CSTR_AT() + CSTR_AT_CW()  + "AUTOCONN=0";
        
    }
    static const String CSTR_AT_CIPSTATUS(void)
    {
        return CSTR_AT() + CSTR_AT_CIP() + "STATUS";
        
    }
    static const String CSTR_AT_CIPCLOSE(void)
    {
        return CSTR_AT() + CSTR_AT_CIP() + "CLOSE";
        
    }
    static const String CSTR_AT_CWLAP(void)
    {
        return CSTR_AT() + CSTR_AT_CW() + "LAP";
        
    }
    static const String CSTR_AT_CIPSEND(void)
    {
        return CSTR_AT() + CSTR_AT_CIP() + "SEND=";
        
    }
    static const String CSTR_AT_OK(void)
    {
        return "OK";
        
    }
    static const String CSTR_AT_SENDOK(void)
    {
        return "SEND " + CSTR_AT_OK();
        
    }  
};
#endif // _CONSTANTS_H
