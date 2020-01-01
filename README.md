
# Inteli-house
Example projects for  SmartHome solution from [Inteli-House]

## Description

This application is used with Arduino based hardware devices for controlling and monitoring your home.
It is complete solution user interaction app, which allows you to focus on hardware and it's functionality.
Once hardware is ready, you will be able to configure the user interface according to your needs.

Solution support there hardware specs:
- up to 10 devices (controllers) per one user account
- up to 8 digital outputs / per device
- up to 8 digital inputs / per device;
- up to 5 analog (16 bit) inputs /per device

All device inputs/outputs are configurable, and can be set-up by user for best operation.
It can be used for following functionalities: 
- Thermostat function heating or cooling, current temperature and setpoint
- Plugs - switch button (on/off)
- Lights - switch button ( on/off)
- Rollers (available by pairing 2 outputs up/down, and defining off delay for endpoint)
- Appliances - switch button (on/off)
- Other -available as switch button (on/off) or push button (with off delay)
- Various settings

### Timers
Automate your digital/analog outputs using programmable timers for each day of week (up to 20 unique state changes can be added per output)
  
### Alarms
Receive push/email messages per analog/digital input change
## Contents
Following repo contains three demo projects:
1) **nano_esp8266_sketch** - arduino nano or similar communication with ESP8266 module using AT commands set
2) **esp8266_sketch** - native esp8266 (NodeMCU,SparkFun, Wemos,  etc.) communication example using Arduino IDE
3) **esp32_sketch** - native esp32 (NodeMCU-32S,SparkFun ESP32, Wemos ESP32, etc.) communication example using Arduino IDE

## Communication specification
UDP protocol is used between client (your hardware) and the server. The exchange mechanism it's basic request/response  UDP datagram, initiated by the client with loop delay 1 seconds or more. That is the reason that you do not need to setup DNS forwarding on your router or worry about WAN IP address changes.

### Settings and login parameters

First you need to setup connection parameters in ***constants.h*** 

#define SSID                "WIFI SSID"             *// change to your WIFI network name*
#define PASSW               "WIFI PASSWORD"         *// change to your WIFI password*
#define DEST_IP             "SERVER DOMAIN NAME"    *// change to server domain name (obtained via mobile app)*
#define DEST_PORT           27735					*// UDP port
#define DEVICE_ADDRESS      0x01					*// device adders, range is 0x01 - 0x0A
//up to 10 devices can be used per user account)

### Communication protocol
Once connection is established ***communication.cpp*** is used for preparing udp request datagram and parse received data. All data transmitted and received are based64 encoded.
All double values are assumed to have one decimal point and are converted to Int16 value using following expression: 
**(double_value)x10.f** -for example if the temperature is 71.6°F, then calculated integer value is 716

### 1.  Request specification

#### Header data(2 bytes):
**byte [0]  = 0x55** 							*//header byte used for delimiter
**byte [1]  = 0x55** 							*//header byte used for delimiter

####  Payload data (23 bytes):
**byte [0-7]  = registrationID** 				*//unique user registration ID 8 bytes. (obtained via mobile app)*
**byte[8]	  = protocolVersion**     			*//protocol version, fixed value of 0x11*
**byte[9] 	  = functionStream**				*//basic function, fixed value of 0x01*
**byte[10]	  = temperatureHighNibble** 		*//temperature sensor high nibble*
**byte[11]    = temperatureLowNibble**  		*//temperature sensor  low nibble*
**byte[12]    = digitalInputAggregatedBits**	*//digital input bits 0-7*
**byte[13]    = analogInput1HighNibble**   		*//analog input 1 high nibble*
**byte[14]    = analogInput1LowNibble**         *//analog input 1 low nibble*
**byte[15]    = analogInput2HighNibble**    	*//analog input 2 high nibble*
**byte[16]    = analogInput2LowNibble**         *//analog input 2 low nibble*
**byte[17]    = analogInput3HighNibble**   	    *//analog input 3 high nibble*
**byte[18]    = analogInput3LowNibble**         *//analog input 3 low nibble*
**byte[19]    = analogInput4HighNibble**   	    *//analog input 4 high nibble*
**byte[20]    = analogInput4LowNibble**         *//analog input 4 low nibble*
**byte[21]    = outputAggregatedBits**			*//digital output bits 0-7*
**byte[22]    = Crc8**					     	*//CRC 8 calculation of the payload data*

####  Complete request datagram:
Full data request contains:  2 header bytes + base64(payload) + Null character  
for ex.  {0x55, 0x55, base64(payload), '\0'}

### 2. Response specification

####  Payload data (10 bytes):
**byte[0] 	 = functionStream**					*//basic function, fixed value of 0x01*
**byte[1]    = outputAggregatedBits**			*//digital output bits 0-7*
**byte[2]    = settingsAggregatedBits**			*//aggregated state heatEnabled = bit 0, tempInvert = bit 1*
**byte[3]    = setpointHighNibble**				*//temperatue setpoing high nibble*
**byte[4]    = setpointLowNibble**				*//temperature setpoing low nibble*
**byte[5]    = hysteresisHighNibble**			*//temperature hysteresis high nibble*
**byte[6]    = hysteresisLowNibble**			*//temperature hysteresis low nibble*
**byte[7]    = tempOffsetHighNibble**			*//temperature offset high nibble*
**byte[8]    = tempOffsetLowNibble**			*//temperature offset low nibble*
**byte[9]    = Crc8**					     	*//CRC 8 calculation of the payload data*

####  Complete response datagram:
Full data response contains:   base64(payload)

**note that there is no delimiter bytes {0x55,0x55} in the response datagram, nor Null character at the end.*


  [Inteli-House]: <http://inteli-house.com>