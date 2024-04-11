#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <debug.h>
#include <SD.h>
#include <rest.h>
#include <database.h>

#define SERVER_HTTP_PORT 80

class DTTSServer {
    public:
        DTTSServer(Database *db);
        void begin();

    private:
        AsyncWebServer server;
        DTTSRestApi rest_api;
};

#endif