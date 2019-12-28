/*
 Copyright (c) 2019 Inteli-House.
 See LICENSE for more information
 */
#define DEBUG_CONSOLE

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

/* CONNECTION PARAMETERS -BEGIN*/
#define SSID                "WIFI SSID"             // change to your WIFI network name
#define PASSW               "WIFI PASSWORD"         // change to your WIFI password
#define DEST_IP             "SERVER DOMAIN NAME"    // change to server domain name (obtained via mobile app)
#define DEST_PORT           27735
#define DEVICE_ADDRESS      0x01
static const char registrationID[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  //change to your unique registration ID (obtained via mobile app)
/* CONNECTION PARAMETERS -END*/

#define INPUT_MAPPING_ONE_WIRE_BUS A0
#define OUTPUT_MAPPING_OUT1 A3 
#define OUTPUT_MAPPING_OUT2 A4 
#define OUTPUT_MAPPING_OUT3 A5 
#define OUTPUT_MAPPING_OUT4 A6 
#define OUTPUT_MAPPING_OUT5 A7 
#define OUTPUT_MAPPING_OUT6 A10 
#define OUTPUT_MAPPING_OUT7 A11
#define OUTPUT_MAPPING_OUT8 A12

#define EEPROM_INITIALIZED_VALUE 18345
#define EEPROM_WIFI_INIT_VALUE 10392
#define EEPROM_DEVICE_INIT_ADDR 0
#define EEPROM_WIFI_INIT_ADDR 4
#define EEPROM_RELAY_INIT_ADDR1 100
#define EEPROM_RELAY_INIT_ADDR2 104
#define EEPROM_RELAY_INIT_ADDR3 108
#define EEPROM_RELAY_INIT_ADDR4 112
#define EEPROM_RELAY_INIT_ADDR5 116
#define EEPROM_RELAY_INIT_ADDR6 120
#define EEPROM_RELAY_INIT_ADDR7 124
#define EEPROM_RELAY_INIT_ADDR8 128

#define EEPROM_MODE_VALUE_ADDR 132
#define EEPROM_TEMP_SETPOINT_ADDR 136
#define EEPROM_HISTERESIS_VALUE_ADDR 140
#define EEPROM_TEMP_OFFSET_VALUE_ADDR 144

#define REC_SIZE  20
#define SEND_SIZE 24

#define CONNECTION_TRIES 8
#define RECONNECTION_TRIES 4
#define FUNC_STREAM 0x01 //basic streaming function


#define DEBUG_CONSOLE //enables debug mode for logging

static int GLOBAL_REPEAT_ERROR_COUNT = 10;
static int GLOBAL_WIFI_REPEAT_ERROR_COUNT = 2;
static int GLOBAL_SERVER_REPEAT_ERROR_COUNT = 2;

static bool packetReceivedCRC_OK = false;

class Constants {
  
public:
   
    static char PROT_VER(void) // protocol version
    {
        static char ch = {0x11};
        
        return ch;
    }
    static char *startSeq(void) //start of sequence = 2 bytes
    {
        static char ch[2] = {0x55, 0x55};
        
        return ch;
    }
    static char *resultDecoded(void) //result from server
    {
        static char ch[REC_SIZE];
        
        return ch;
    }
    static  char NullChar(void)
    {
        return '\0';
        
    } 
};
#endif // _CONSTANTS_H
