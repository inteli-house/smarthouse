#include "sensors.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "constants.h"

OneWire oneWire(INPUT_MAPPING_ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void initializeSensors() {
    sensors.begin(); 
}

float getTemperature () {
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    return temp;
}

uint8_t crc8(const uint8_t *addr, uint8_t len)
{
    return oneWire.crc8(addr,len);
}
