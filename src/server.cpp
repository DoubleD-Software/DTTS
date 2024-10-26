#include <server.h>

/**
 * This file contains the top level server implementation that interacts with the rest api and websockets
 

 * Constructor for the DTTSServer class.
 * @param db The database object to use for the server.
*/
DTTSServer::DTTSServer(Database *db, OLED *oled, TM1637 *num_disp) : server(SERVER_HTTP_PORT), ws(WS_URL), run_handler(db, &ws, oled, num_disp), rest_api(db, &run_handler) {
}

/**
 * Starts the server and registers its endpoints with the rest_api object.
*/
void DTTSServer::begin() {
    DEBUG_SER_PRINTLN("Starting server...");

    run_handler.init();

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
    server.on("/api/gradingkeys", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.getGradingKeys(request);
    });
    server.on("/api/gradingkeys", HTTP_DELETE, [&](AsyncWebServerRequest *request) {
        rest_api.deleteGradingKey(request);
    });
    server.on("/api/gradingkeys",
        HTTP_PUT,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.putGradingKey(request, data_str);
        }
    );
    server.on("/api/gradingkeys",
        HTTP_PATCH,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.patchGradingKey(request, data_str);
        }
    );
    server.on("/api/classes", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.getClasses(request);
    });
    server.on("/api/classes", HTTP_DELETE, [&](AsyncWebServerRequest *request) {
        rest_api.deleteClass(request);
    });
    server.on("/api/classes",
        HTTP_PUT,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.putClass(request, data_str);
        }
    );
    server.on("/api/classes",
        HTTP_PATCH,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.patchClass(request, data_str);
        }
    );
    server.on("/api/teachers", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.getTeachers(request);
    });
    server.on("/api/teachers", HTTP_DELETE, [&](AsyncWebServerRequest *request) {
        rest_api.deleteTeacher(request);
    });
    server.on("/api/teachers",
        HTTP_PUT,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.putTeacher(request, data_str);
        }
    );
    server.on("/api/teachers",
        HTTP_PATCH,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.patchTeacher(request, data_str);
        }
    );
    server.on("/api/whoami", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.whoAmI(request);
    });
    server.on("/api/login",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            rest_api.authenticate(request, data_str);
        }
    );
    server.on("/api/active", HTTP_GET, [&](AsyncWebServerRequest *request) {
        rest_api.getActive(request);
    });
    server.on("/api/admin/studentsReset",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            bool resetted = rest_api.resetStudents(request, data_str);

            if (resetted) {
                server.end();
                DEBUG_SER_PRINTLN("Student reset completed. Restarting...");
                esp_restart();
            }
        }
    );
    server.on("/api/admin/factoryReset",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {request->send(400);},
        NULL,
        [&](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            String data_str = "";
            for (int i = 0; i < len; i++) {
                data_str += (char)data[i];
            }
            bool resetted = rest_api.factoryReset(request, data_str);

            if (resetted) {
                server.end();
                DEBUG_SER_PRINTLN("Factory reset completed. Restarting...");
                esp_restart();
            }
        }
    );

    String htmlEndpoints[] = {
        "/runs/view", "/runs/edit", "/runs/new", "/runs", 
        "/students/view", "/students/edit", "/students/new", "/students", 
        "/classes/view", "/classes/edit", "/classes/new", "/classes", 
        "/gradingkeys/view", "/gradingkeys/edit", "/gradingkeys/new", "/gradingkeys", 
        "/teachers/view", "/teachers/edit", "/teachers/new", "/teachers",
        "/active/tags", "/active/dash",
    };

    for (String endpoint : htmlEndpoints) {
        String endpointPath = "/website" + endpoint + ".html";
        server.serveStatic(endpoint.c_str(), LittleFS, endpointPath.c_str()).setCacheControl("max-age=604800");
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/website/index.html", "text/html");
    });
    server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    server.serveStatic("/_app", LittleFS, "/website/_app").setCacheControl("max-age=2592000");
    server.serveStatic("/DigitalNumbers-reduced.ttf", LittleFS, "/website/DigitalNumbers-reduced.ttf").setCacheControl("max-age=2592000");
    server.serveStatic("/manifest.json", LittleFS, "/website/manifest.json").setCacheControl("max-age=2592000");
    server.serveStatic("/app_icon.webp", LittleFS, "/website/app_icon.webp").setCacheControl("max-age=2592000");
    server.serveStatic("/favicon.webp", LittleFS, "/website/favicon.webp").setCacheControl("max-age=2592000");
    server.serveStatic("/dtts.webp", LittleFS, "/website/dtts.webp").setCacheControl("max-age=2592000");

    server.addHandler(&ws);

    server.begin();
    DEBUG_SER_PRINTLN("Server started.");
}

void DTTSServer::handleRunLogic() {
    run_handler.handle();
}