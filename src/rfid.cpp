#include <rfid.h>

RFID::RFID() {
    
}

void RFID::begin() {
    DEBUG_SER_PRINTLN("Initializing RFID module...");
    Serial1.begin(RFID_BAUD);

    cmd_read_multi = new uint8_t[10];
    cmd_stop_multi = new uint8_t[7];

    const uint8_t read_multi[10] = {0XAA, 0X00, 0X27, 0X00, 0X03, 0X22, 0XFF, 0XFF, 0X4A, 0XDD};
    const uint8_t stop_multi[7] = {0XAA, 0X00, 0X28, 0X00, 0X00, 0X28, 0XDD};

    memcpy(cmd_read_multi, read_multi, 10);
    memcpy(cmd_stop_multi, stop_multi, 7);

    stop_read_multi();
    DEBUG_SER_PRINTLN("RFID module initialized.");
}

void RFID::start_read_multi() {
    Serial1.write(cmd_read_multi, 10);
}

void RFID::stop_read_multi() {
    Serial1.write(cmd_stop_multi, 7);
}