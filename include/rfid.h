#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <debug.h>

#define RFID_BAUD 115200

class RFID {
    public:
        RFID();
        void begin();
        void start_read_multi();
        void stop_read_multi();
    
    private:
        uint8_t *cmd_read_multi;
        uint8_t *cmd_stop_multi;
};

#endif