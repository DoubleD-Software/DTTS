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
        void getRun(AsyncWebServerRequest *request);
        void deleteRun(AsyncWebServerRequest *request);
        void putRun(AsyncWebServerRequest *request, String data);
        void getStudent(AsyncWebServerRequest *request);
        void deleteStudent(AsyncWebServerRequest *request);
        void putStudent(AsyncWebServerRequest *request, String data);
        void patchStudent(AsyncWebServerRequest *request, String data);
        
    private:
        Database *db;
        int last_run_id = 0;
};

#endif