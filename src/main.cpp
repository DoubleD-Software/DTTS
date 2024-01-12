#include <Arduino.h>
#include <definitions.h>
#include <debug.h>
#include <fatal.h>
#include <rfid.h>
#include <oled.h>
#include <tm1637.h>
#include <main.h>
#include <SD.h>
#include <sql.h>

RFID rfid;
OLED oled(OLED_SDA, OLED_SCL, OLED_WIDTH, OLED_HEIGHT);
TM1637 num_disp(NUM_DISP_CLK, NUM_DISP_DIO, 6);
SQL sql;

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

    oled.begin();
    rfid.begin();
    rfid.setWorkArea(RFID_REGION);
    num_disp.begin();
    
    DEBUG_SER_PRINTLN("Initializing SD card.");
    if(SD.begin(SD_CS)) {
        DEBUG_SER_PRINTLN("SD card initialized.");
    } else {
        DEBUG_SER_PRINTLN("SD card initialization failed.");
        sysHalt(4, "SD card failed.");
    }

    sql_column_descriptor_t columns[2];
    columns[0].name = "id";
    columns[0].type = SQL_TYPE_INT;
    columns[1].name = "name";
    columns[1].type = SQL_TYPE_VARCHAR;

    sql_column_t values[2];
    values[0].name = "id";
    values[0].type = SQL_TYPE_INT;
    values[0].value_int = 1;
    values[1].name = "name";
    values[1].type = SQL_TYPE_VARCHAR;
    values[1].value_varchar = (char*) "Arnold Schwarzenegger";

    sql_column_t select[2];
    select[0].name = "id";
    select[0].type = SQL_TYPE_INT;
    select[1].name = "name";
    select[1].type = SQL_TYPE_VARCHAR;

    sql_column_t where[2];
    where[0].name = "id";
    where[0].type = SQL_TYPE_INT;
    where[0].value_int = 1;
    where[1].name = "name";
    where[1].type = SQL_TYPE_VARCHAR;
    where[1].value_varchar = (char*) "Arnol Schwarzenegger";

    sql.begin("/sd/DTTS.db");
    sql.dropTable("teachers");
    sql.createTable("teachers", columns, 2);
    sql.insert("teachers", values, 2);
    sql.find("teachers", select, where, 2, 2);

    DEBUG_SER_PRINT("ID: ");
    DEBUG_SER_PRINTLN(select[0].value_int);
    DEBUG_SER_PRINT("Name: ");
    DEBUG_SER_PRINTLN(select[1].value_varchar);

    oled.clear();
    oled.print("DTTS\n", 2);
    oled.print("DoubleD Software\n", 1);
    oled.print(VERSION, 1);
    num_disp.displayString("ddsoft,");
    DEBUG_SER_PRINTLN("Initialized DTTS.");
}

void loop() {

}