#include <Arduino.h>
#include <LittleFS.h>
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
#include <integrity.h>
#include <ESPAsyncDNSServer.h>

OLED oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT);
TM1637 num_disp(NUM_DISP_CLK, NUM_DISP_DIO, 6);
Database db("/sd/dtts.db");
DTTSServer server(&db, &oled, &num_disp);
IPAddress local_ip(192, 168, 0, 1);
IPAddress subnet_mask(255, 255, 255, 0);
AsyncDNSServer dns_server;

void setup() {
    DEBUG_SER_INIT(115200);
    DEBUG_SER_PRINTLN("Initializing DTTS...");

    oled.begin();
    num_disp.begin();
    
    DEBUG_SER_PRINTLN("Initializing SD card...");
    if(SD.begin(SD_CS, SPI, 80000000, "/sd", 8)) {
        DEBUG_SER_PRINTLN("SD card initialized.");
    } else {
        DEBUG_SER_PRINTLN("SD card initialization failed.");
        sysHalt(SD_FAILED, "SD card failed.");
    }

    DEBUG_SER_PRINTLN("Initializing LittleFS...");
    if(LittleFS.begin()) {
        DEBUG_SER_PRINTLN("LittleFS initialized.");
    } else {
        DEBUG_SER_PRINTLN("LittleFS initialization failed.");
        sysHalt(RECOVERY_FAILED, "LittleFS failed.");
    }

    oled.clear();
    oled.print("DTTS\n", 2);
    oled.print("DoubleD Software\n", 1);
    oled.print(String(VERSION + String("\n")).c_str(), 1);
    oled.print("System startet...", 1);
    num_disp.displayString("ddsoft,");

    DEBUG_SER_PRINTLN("Performing integrity check...");
    performIntegrityCheck("/website", "/website");
    DEBUG_SER_PRINTLN("Integrity check complete.");
    DEBUG_SER_PRINTLN("Deleting extraneous files...");
    deleteExtraneousFiles("/website", "/website");
    DEBUG_SER_PRINTLN("Extraneous files deleted.");

    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.softAPConfig(local_ip, local_ip, subnet_mask);
    WiFi.setHostname("DTTS");
    WiFi.enableLongRange(true);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, 1, 0, 1);

    db.open();
    server.begin();

    dns_server.setErrorReplyCode(AsyncDNSReplyCode::ServerFailure);

    DEBUG_SER_PRINTLN("Starting DNS Server...");
    dns_server.start(DNS_PORT, "*", local_ip);
    DEBUG_SER_PRINTLN("DNS startet. Resolving on *");

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {
    server.handleRunLogic();
}