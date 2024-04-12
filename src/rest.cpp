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
        std::vector<RunInfo> runs = db->getRunsByDate(date);
        if (runs.size() == 0) {
            request_result = 404;
        } else {
            for (int i = 0; i < runs.size(); i++) {
                String index = String(i);
                doc[index]["type"] = runs[i].type;
                doc[index]["length"] = runs[i].length;
                doc[index]["teacher"] = runs[i].teacher;
                doc[index]["class"] = runs[i].class_name;
                doc[index]["avg_grade"] = runs[i].avg_grade;
                doc[index]["avg_time"] = runs[i].avg_time;
            }
        }
    } else if (request->hasParam("student") && request->hasParam("id")) {
        
    } else if (request->hasParam("id")) {

    } else {

    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}