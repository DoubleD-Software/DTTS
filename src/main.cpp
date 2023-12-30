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

void setup() {
    DEBUG_SER_INIT(115200);
    DEBUG_SER_PRINTLN("Initializing DTTS...");

    rfid.begin();
    oled.begin();
    num_disp.begin();

    oled.clear();
    oled.print("DTTS");
    num_disp.displayNumBuffer((uint8_t *) "\1\2\3\4\5\6");

    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {
  
}