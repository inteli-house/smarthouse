#include "communication.h"
#include "constants.h"
#include "sensors.h"
#include "cBase64.h"
#include <EepromUtil.h>


static char  sendBuff [SEND_SIZE];
static char  sendBuffEncoded [2 + SEND_SIZE + 8 + 1];

int histeresis = 5;
int lastHisteresis = 5;
int setPoint = 220;
int lastSetPoint = 220;
int tempOffset = 0;
int lastTempOffset = 0;
bool heatEnabled = false;
bool tempInvert = false;
int modeAgregate = 0;
int lastModeAgregate = 0;
bool heatOn = false;
bool lastheatOn = false;
int temperature;
bool out1, out2, out3, out4, out5, out6, out7, out8;
int analog1;
int analog2 = 100;
int analog3;
int analog4;
int counter = 0;
int addition = 1;
bool digitalInp1, digitalInp2, digitalInp3, digitalInp4, digitalInp5, digitalInp6, digitalInp7, digitalInp8;

int BitShiftCombine( unsigned char x_high, unsigned char x_low);
void setOutputs();

char *requestSend(bool firstStart, bool wasDisconnected) { //prepare send byte array for server transmission
  int i = 0;
  for (int j = 0; j < 8 ; j++) {
    sendBuff[i++] =  registrationID[j];     //unique user registration ID 8 bytes.
  }
  sendBuff[i++] = DEVICE_ADDRESS;            //device address (0x01 - 0x0A)
  sendBuff[i++] = Constants::PROT_VER();     //protocol version
  sendBuff[i++] = FUNC_STREAM;               //basic function
  sendBuff[i++] = (temperature >> 8) & 0xFF; //temperature sensor high nibble
  sendBuff[i++] = temperature & 0xFF;        //temperature sensor  low nibble
  sendBuff[i++] = (digitalInp8 << 7) | (digitalInp7 << 6) | (digitalInp6 << 5) | (digitalInp5 << 4) | (digitalInp4 << 3) | (digitalInp3 << 2) | (digitalInp2 << 1) | (digitalInp1); // digital input bits 0-7
  sendBuff[i++] = (analog1 >> 8) & 0xFF;    //analog input 1 high nibble
  sendBuff[i++] = analog1 & 0xFF;           //analog input 1 low nibble
  sendBuff[i++] = (analog2 >> 8) & 0xFF;    //analog input 2 high nibble
  sendBuff[i++] = analog2 & 0xFF;           //analog input 2 low nibble
  sendBuff[i++] = (analog3 >> 8) & 0xFF;    //analog input 3 high nibble
  sendBuff[i++] = analog3 & 0xFF;           //analog input 3 low nibble
  sendBuff[i++] = (analog4 >> 8) & 0xFF;    //analog input 4 high nibble
  sendBuff[i++] = analog4 & 0xFF;           //analog input 4 low nibble
  sendBuff [i++] = (out8 << 7) | (out7 << 6) | (out6 << 5) | (out5 << 4) | (out4 << 3) | (out3 << 2) | (out2 << 1) | (out1); //digital output bits 0-7

  sendBuff [i++] = crc8((uint8_t*)sendBuff, i - 1 );

  int encodedLen = base64_enc_len(SEND_SIZE);
  sendBuffEncoded[0] = Constants::startSeq()[0];
  sendBuffEncoded[1] = Constants::startSeq()[1];
  base64_encode(sendBuffEncoded + 2, sendBuff, SEND_SIZE);
  sendBuffEncoded[encodedLen + 2] = Constants::NullChar();

  return sendBuffEncoded;
}

int parseReceived(char *resultDecoded) { //parse received array from server

  switch  (resultDecoded[0]) {
    case FUNC_STREAM:

      out2 = resultDecoded[1] >> 1 & 0x01;
      out3 = resultDecoded[1] >> 2 & 0x01;
      out4 = resultDecoded[1] >> 3 & 0x01;
      out5 = resultDecoded[1] >> 4 & 0x01;
      out6 = resultDecoded[1] >> 5 & 0x01;
      out7 = resultDecoded[1] >> 6 & 0x01;
      out8 = resultDecoded[1] >> 7 & 0x01;

      heatEnabled = resultDecoded[2] & 0x01;
      tempInvert = (resultDecoded[2] >> 1) & 0x01;
      modeAgregate = BitShiftCombine (resultDecoded [3],  resultDecoded[2]);
      setPoint  = BitShiftCombine (resultDecoded [4],  resultDecoded[5]);
      histeresis  = BitShiftCombine (resultDecoded [6],  resultDecoded[7]);
      tempOffset  = BitShiftCombine (resultDecoded [8],  resultDecoded[9]);

      setOutputs();
      break;

    default:
      return -1;
      break;
  }
  return 0;
}
void setOutputs() {
  if (!heatEnabled) {
    digitalWrite(OUTPUT_MAPPING_OUT1, out1);
  }
  digitalWrite(OUTPUT_MAPPING_OUT2, out2);
  digitalWrite(OUTPUT_MAPPING_OUT3, out3);
  digitalWrite(OUTPUT_MAPPING_OUT4, out4);
  digitalWrite(OUTPUT_MAPPING_OUT5, out5);
  digitalWrite(OUTPUT_MAPPING_OUT6, out6);
  digitalWrite(OUTPUT_MAPPING_OUT7, out7);
  digitalWrite(OUTPUT_MAPPING_OUT8, out8);
}

int BitShiftCombine( unsigned char x_high, unsigned char x_low)
{
  int combined;
  combined = x_high;              // send x_high to rightmost 8 bits
  combined = combined << 8;       // shift x_high over to leftmost 8 bits
  combined |= x_low;              // logical OR keeps x_high intact in combined and fills in rightmost 8 bits
  return combined;
}
