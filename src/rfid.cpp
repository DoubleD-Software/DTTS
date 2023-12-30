#include <rfid.h>


RFID::RFID() {   
}

/**
 * Initialize the RFID module. This function has to be called before any other function of this class.
*/
void RFID::begin() {
    DEBUG_SER_PRINTLN("Initializing RFID module...");
    Serial1.begin(RFID_BAUD);

    cmd_read_multi = new uint8_t[10]; // Allocate memory for the command arrays.
    cmd_stop_multi = new uint8_t[7];

    const uint8_t read_multi[10] = {0XAA, 0X00, 0X27, 0X00, 0X03, 0X22, 0XFF, 0XFF, 0X4A, 0XDD};
    const uint8_t stop_multi[7] = {0XAA, 0X00, 0X28, 0X00, 0X00, 0X28, 0XDD};

    memcpy(cmd_read_multi, read_multi, 10); // Copy the command arrays to the class variables.
    memcpy(cmd_stop_multi, stop_multi, 7);

    stopReadMulti(); // Stop any ongoing multi read.
    DEBUG_SER_PRINTLN("RFID module initialized.");
}

/**
 * Start the RFID module in multi read mode.
*/
void RFID::startReadMulti() {
    Serial1.write(cmd_read_multi, 10);
}

/**
 * Stop the RFID module from multi read mode.
*/
void RFID::stopReadMulti() {
    Serial1.write(cmd_stop_multi, 7);
}