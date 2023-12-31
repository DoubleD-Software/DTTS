#include <Arduino.h>
#include <definitions.h>
#include <debug.h>
#include <fatal.h>
#include <rfid.h>
#include <oled.h>
#include <tm1637.h>

RFID rfid;
OLED oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT);
TM1637 num_disp(NUM_DISP_CLK, NUM_DISP_DIO, 6);

void rfidCallback(uint8_t* tag) {
    DEBUG_SER_PRINT("Tag: ");
    for (int i = 0; i < 12; i++) {
        DEBUG_SER_PRINT(tag[i], HEX);
        DEBUG_SER_PRINT(" ");
    }
    DEBUG_SER_PRINTLN();
}

void setup() {
    DEBUG_SER_INIT(115200);
    DEBUG_SER_PRINTLN("Initializing DTTS...");

    rfid.begin();
    oled.begin();
    num_disp.begin();

    oled.clear();
    oled.print("DTTS\n", 2);
    oled.print("DoubleD Software\n", 1);
    oled.print(VERSION, 1);
    num_disp.displayString("ddsoft,");

    rfid.startReadMulti(rfidCallback);

    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {

}