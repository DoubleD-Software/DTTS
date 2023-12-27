#include <Arduino.h>
#include <definitions.h>
#include <debug.h>
#include <fatal.h>
#include <rfid.h>
#include <oled.h>

RFID rfid;
OLED oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT);

void setup() {
    DEBUG_SER_INIT(115200);
    DEBUG_SER_PRINTLN("Initializing DTTS...");

    rfid.begin();
    oled.begin();

    oled.clear();
    oled.print("DTTS");

    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {
  
}