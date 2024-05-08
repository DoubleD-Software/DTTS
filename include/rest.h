#ifndef REST_H
#define REST_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <database.h>

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
        void getGradingKeys(AsyncWebServerRequest *request);
        void deleteGradingKey(AsyncWebServerRequest *request);
        void putGradingKey(AsyncWebServerRequest *request, String data);
        void patchGradingKey(AsyncWebServerRequest *request, String data);
        void getClasses(AsyncWebServerRequest *request);
        void deleteClass(AsyncWebServerRequest *request);
        void putClass(AsyncWebServerRequest *request, String data);
        void patchClass(AsyncWebServerRequest *request, String data);
        
    private:
        Database *db;
        int last_run_id = 0;
};

#endif