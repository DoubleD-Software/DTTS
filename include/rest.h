#ifndef REST_H
#define REST_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <database.h>

#define SPRINT_TYPE 0
#define RUN_TYPE 1
#define GENDER_TYPE_MALE 0
#define GENDER_TYPE_FEMALE 1

class DTTSRestApi {
    public:
        DTTSRestApi(Database *db);
        void runHandler(AsyncWebServerRequest *request);

    private:
        Database *db;
        void getRun(AsyncWebServerRequest *request);
        void deleteRun(AsyncWebServerRequest *request);
        void putRun(AsyncWebServerRequest *request);
};

#endif