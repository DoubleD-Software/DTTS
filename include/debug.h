#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

#define DEBUG

#ifdef DEBUG
    #define DEBUG_SER_INIT(baudrate) Serial.begin(baudrate)
    #define DEBUG_SER_PRINTLN(...) Serial.println(__VA_ARGS__)
    #define DEBUG_SER_PRINT(...) Serial.print(__VA_ARGS__)
#else
    #define DEBUG_SER_INIT(baudrate)
    #define DEBUG_SER_PRINTLN(...)
    #define DEBUG_SER_PRINT(...)
#endif

#endif