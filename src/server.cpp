#include <server.h>

DTTSServer::DTTSServer(Database *db) : server(SERVER_HTTP_PORT), rest_api(db) {
}

void DTTSServer::begin() {
    DEBUG_SER_PRINTLN("Starting server...");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/web/index.html", "text/html");
    });
    server.on("/api/runs", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.getRun(request);
    });
    server.on("/api/runs", HTTP_DELETE, [&](AsyncWebServerRequest *request) {
        rest_api.deleteRun(request);
    });
    server.on("/api/runs",
        HTTP_PUT,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.putRun(request, data_str);
        }
    );
    server.on("/api/students", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.getStudent(request);
    });
    server.on("/api/students", HTTP_DELETE, [&](AsyncWebServerRequest *request) {
        rest_api.deleteStudent(request);
    });
    server.on("/api/students",
        HTTP_PUT,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.putStudent(request, data_str);
        }
    );
    server.on("/api/students",
        HTTP_PATCH,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.patchStudent(request, data_str);
        }
    );
    
    server.serveStatic("/", SD, "/web");
    server.begin();
    DEBUG_SER_PRINTLN("Server started.");
}