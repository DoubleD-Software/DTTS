#ifndef REST_H
#define REST_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <database.h>
#include <globals.h>

#define ACCESS_USER 0
#define ACCESS_ADMIN 1

#define ACCESS_DENIED 0
#define ACCESS_GRANTED 1

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
        void getTeachers(AsyncWebServerRequest *request);
        void deleteTeacher(AsyncWebServerRequest *request);
        void putTeacher(AsyncWebServerRequest *request, String data);
        void patchTeacher(AsyncWebServerRequest *request, String data);
        void whoAmI(AsyncWebServerRequest *request);
        void authenticate(AsyncWebServerRequest *request, String data);
        
    private:
        Database *db;
        int last_run_id = 0;
        int current_session_id = 0;
        bool user_logged_in = false;
        int access_level = ACCESS_USER;

        int checkAuth(AsyncWebServerRequest *request);
};

#endif