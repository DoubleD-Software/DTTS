#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <debug.h>
#include <SD.h>
#include <LittleFS.h>
#include <rest.h>
#include <database.h>
#include <runlogic.h>
#include <oled.h>
#include <tm1637.h>

#define SERVER_HTTP_PORT 80
#define WS_URL "/api/ws"

class DTTSServer {
    public:
        DTTSServer(Database *db, OLED *oled, TM1637 *num_disp);
        void begin();
        void handleRunLogic();

    private:
        AsyncWebServer server;
        AsyncWebSocket ws;
        RunHandler run_handler;
        DTTSRestApi rest_api;
};

#endif