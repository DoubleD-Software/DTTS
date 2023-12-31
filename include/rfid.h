#ifndef RFID_H
#define RFID_H

#include <Arduino.h>
#include <debug.h>

#define RFID_BAUD 115200
#define RESP_TIMEOUT 50

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
    uint8_t type = 0x01;
    uint8_t command = 0x02;
    uint8_t pl_h = 0x00;
    uint8_t pl_l = 0x00;
    uint8_t param = 0x00;
    uint8_t checksum = 0x00;
    uint8_t end = 0xDD;
} rfid_resp_stop_multi_t;

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

typedef struct {
    uint8_t header = 0xAA;
    uint8_t type = 0x02;
    uint8_t command = 0x22;
    uint8_t pl_h = 0x00;
    uint8_t pl_l = 0x11;
    uint8_t rssi = 0x00;
    uint8_t pc_h;
    uint8_t pc_l;
    uint8_t epc[12];
    uint8_t crc_h;
    uint8_t crc_l;
    uint8_t checksum = 0x00;
    uint8_t end = 0xDD;
} rfid_tag_resp_t;

class RFID {
    public:
        RFID();
        void begin();
        void startReadMulti(void (*callbackFunction)(uint8_t*));
        void stopReadMulti();
    
    private:
        TaskHandle_t polling_task_handle;
};

void rfidPollingTask(void *pvParameters);

#endif