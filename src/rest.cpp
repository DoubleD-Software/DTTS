    #include <rest.h>

DTTSRestApi::DTTSRestApi(Database *db) {
    this->db = db;
}

void DTTSRestApi::runHandler(AsyncWebServerRequest *request) {
    WebRequestMethodComposite method = request->method();
    if (method == HTTP_GET) {
        getRun(request);
    } else if (method == HTTP_DELETE) {
        deleteRun(request);
    } else if (method == HTTP_PUT) {
        putRun(request);
    }
}

void DTTSRestApi::getRun(AsyncWebServerRequest *request) {
    JsonDocument doc;
    int request_result = 200;

    if (request->hasParam("date")) {
        int date = request->getParam("date")->value().toInt();

    } else if (request->hasParam("student") && request->hasParam("id")) {
        
    } else if (request->hasParam("id")) {

    } else {

    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}