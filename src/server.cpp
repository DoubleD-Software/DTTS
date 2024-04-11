#include <server.h>

DTTSServer::DTTSServer(Database *db) : server(SERVER_HTTP_PORT), rest_api(db) {
}

void DTTSServer::begin() {
    DEBUG_SER_PRINTLN("Starting server...");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SD, "/web/index.html", "text/html");
    });
    server.on("/api/run", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.runHandler(request);
    });
    
    server.serveStatic("/", SD, "/web");
    server.begin();
    DEBUG_SER_PRINTLN("Server started.");
}