#include <Arduino.h>
#include <definitions.h>
#include <debug.h>
#include <fatal.h>
#include <oled.h>
#include <tm1637.h>
#include <main.h>
#include <SD.h>
#include <error_codes.h>
#include <server.h>
#include <database.h>
#include <runlogic.h>

OLED oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT);
TM1637 num_disp(NUM_DISP_CLK, NUM_DISP_DIO, 6);
Database db(DB_PATH);
DTTSServer server(&db, &oled, &num_disp);

#define SSID "DTTS"
#define PASSWORD "doubledsoftware"

void setup() {
    DEBUG_SER_INIT(115200);
    DEBUG_SER_PRINTLN("Initializing DTTS...");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 0, 1), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(SSID, PASSWORD, 1, 0, 1);

    oled.begin();
    num_disp.begin();
    
    DEBUG_SER_PRINTLN("Initializing SD card.");
    if(SD.begin(SD_CS, SPI, 80000000, "/sd", 8)) {
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

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {
    server.handleRunLogic();
}