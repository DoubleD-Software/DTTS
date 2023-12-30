#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <debug.h>

#define RFID_BAUD 115200

class RFID {
    public:
        RFID();
        void begin();
        void startReadMulti();
        void stopReadMulti();
    
    private:
        uint8_t *cmd_read_multi;
        uint8_t *cmd_stop_multi;
};

#endif