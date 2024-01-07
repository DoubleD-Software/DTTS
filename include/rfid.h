#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <debug.h>

#define RFID_BAUD 115200

#define START_BYTE 0x02
#define COMMAND_BYTE 0x22
#define EPC_START_OFFSET 9
#define EPC_END_OFFSET 20

typedef struct {
    uint8_t header = 0xAA;
    uint8_t type = 0x00;
    uint8_t command = 0x28;
    uint8_t pl_h = 0x00;
    uint8_t pl_l = 0x00;
    uint8_t checksum = 0x28;
    uint8_t end = 0xDD;
} rfid_cmd_stop_multi_t;

typedef struct {
    uint8_t header = 0xAA;
    uint8_t type = 0x00;
    uint8_t command = 0x27;
    uint8_t pl_h = 0x00;
    uint8_t pl_l = 0x03;
    uint8_t reserved = 0x22;
    uint8_t cnt_h = 0xFF;
    uint8_t cnt_l = 0xFF;
    uint8_t checksum = 0x4A;
    uint8_t end = 0xDD;
} rfid_cmd_start_multi_t;

class RFID {
    public:
        RFID();
        void begin();
        void startReadMulti(void (*callback_function)(uint8_t*, uint8_t));
        void stopReadMulti();
    
    private:
        TaskHandle_t polling_task_handle;
};

void rfidPollingTask(void *pv_parameters);

#endif