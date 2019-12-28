/*
 * Copyright (c) 2019 Inteli-House.
 * See LICENSE for more information
 */
#ifndef _SENSORS_H
#define _SENSORS_H

#define ONE_WIRE_BUS A0

#include <Arduino.h>

void initializeSensors();
float getTemperature ();

uint8_t crc8(const uint8_t *addr, uint8_t len);
#endif // _SENSORS_H
