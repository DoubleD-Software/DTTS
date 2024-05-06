#include <Arduino.h>
#include <definitions.h>
#include <debug.h>
#include <fatal.h>
#include <rfid.h>
#include <oled.h>
#include <tm1637.h>
#include <main.h>
#include <SD.h>
#include <error_codes.h>
#include <server.h>
#include <database.h>

RFID rfid;
OLED oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT);
TM1637 num_disp(NUM_DISP_CLK, NUM_DISP_DIO, 6);
Database db(DB_PATH);
DTTSServer server(&db);

#define SSID "DTTS"
#define PASSWORD "doubledsoftware"

void rfidCallback(uint8_t* tag, uint8_t rssi) {
    DEBUG_SER_PRINT("Tag: ");
    for (int i = 0; i < 12; i++) {
        DEBUG_SER_PRINT(tag[i], HEX);
        DEBUG_SER_PRINT(" ");
    }
    DEBUG_SER_PRINTLN();
    DEBUG_SER_PRINT("RSSI: ");
    DEBUG_SER_PRINTLN(rssi);
}

void setup() {
    DEBUG_SER_INIT(115200);
    DEBUG_SER_PRINTLN("Initializing DTTS...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASSWORD);

    oled.begin();
    rfid.begin();
    rfid.setWorkArea(RFID_REGION);
    num_disp.begin();
    
    DEBUG_SER_PRINTLN("Initializing SD card.");
    if(SD.begin(SD_CS)) {
        DEBUG_SER_PRINTLN("SD card initialized.");
    } else {
        DEBUG_SER_PRINTLN("SD card initialization failed.");
        sysHalt(SD_FAILED, "SD card failed.");
    }

    oled.clear();
    oled.print("DTTS\n", 2);
    oled.print("DoubleD Software\n", 1);
    oled.print(VERSION, 1);
    num_disp.displayString("ddsoft,");

    db.open();
    server.begin();

    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {

}