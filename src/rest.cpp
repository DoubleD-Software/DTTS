#include <rest.h>

DTTSRestApi::DTTSRestApi(Database *db) {
    this->db = db;
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
                doc[index]["avg_grade"] = String(runs[i].avg_grade, 2);
                doc[index]["avg_time"] = runs[i].avg_time;
            }
        }
    } else if (request->hasParam("student") && request->hasParam("id")) {
        int student_id = request->getParam("student")->value().toInt();
        int run_id = request->getParam("id")->value().toInt();
        RunInfoStudentLaps run_info = db->getRunInfoStudent(run_id, student_id);
        if (run_info.time == -1) {
            request_result = 404;
        } else if (run_info.time == -2) {
            request_result = 406;
        } else {
            doc["name"] = run_info.student_name;
            doc["time"] = run_info.time;
            doc["grade"] = String(run_info.grade, 2);
            doc["length"] = run_info.length;
            for (int i = 0; i < run_info.laps.size(); i++) {
                String index = String(run_info.laps[i].lap_number);
                doc["rounds"][index]["length"] = run_info.laps[i].length;
                doc["rounds"][index]["time"] = run_info.laps[i].time;
            }
        }   
    } else if (request->hasParam("id")) {
        int run_id = request->getParam("id")->value().toInt();
        RunInfoSpecific run_info = db->getRunInfo(run_id);
        if (run_info.type == -1) {
            request_result = 404;
        } else {
            doc["type"] = run_info.type;
            doc["length"] = run_info.length;
            doc["date"] = run_info.date;
            doc["teacher"] = run_info.teacher;
            doc["class"] = run_info.class_name;
            doc["grading_key_m_id"] = run_info.grading_key_m;
            doc["grading_key_f_id"] = run_info.grading_key_f;
            doc["avg_grade"] = String(run_info.avg_grade, 2);
            doc["avg_time"] = run_info.avg_time;
            for (int i = 0; i < run_info.students.size(); i++) {
                String index = String(i);
                doc["students"][index]["name"] = run_info.students[i].name;
                doc["students"][index]["grade"] = String(run_info.students[i].grade, 2);
            }
        }
    } else {
        request_result = 400;
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}

void DTTSRestApi::deleteRun(AsyncWebServerRequest *request) {
    int request_result = 200;

    if (request->hasParam("id")) {
        int run_id = request->getParam("id")->value().toInt();
        if (db->deleteRun(run_id)) {
            request_result = 404;
        }
    } else {
        request_result = 400;
    }

    request->send(request_result, "application/json", "{}");
}

void DTTSRestApi::putRun(AsyncWebServerRequest *request, String data) {
    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (doc.containsKey("type") && doc.containsKey("length") && doc.containsKey("date") && doc.containsKey("class_id") && doc.containsKey("teacher_id") && doc.containsKey("grading_key_male_id") && doc.containsKey("grading_key_female_id") && doc.containsKey("laps") && doc.containsKey("participants")) {
            std::vector<int> participants;
            for (int i = 0; i < doc["participants"].size(); i++) {
                participants.push_back(doc["participants"][i]);
            }
            int type = doc["type"];
            int length = doc["length"];
            int date = doc["date"];
            int class_id = doc["class_id"];
            int teacher_id = doc["teacher_id"];
            int grading_key_m_id = doc["grading_key_male_id"];
            int grading_key_f_id = doc["grading_key_female_id"];
            int laps = doc["laps"];

            int run_id = db->putRun(type, date, class_id, grading_key_m_id, grading_key_f_id, teacher_id, length, laps, participants);

            if (run_id != DB_FAILED) {
                this->last_run_id = run_id;
            } else {
                request_result = 500;
            }
        } else {
            request_result = 400;
        }
    }

    request->send(request_result, "application/json", "{}");
}

void DTTSRestApi::getStudent(AsyncWebServerRequest *request) {
    int request_result = 200;
    JsonDocument doc;

    if (request->hasParam("id")) {
        int student_id = request->getParam("id")->value().toInt();
        StudentInfo student_info = db->getStudentInfo(student_id);
        if (student_info.student_name == "") {
            request_result = 404;
        } else {
            doc["name"] = student_info.student_name;
            doc["gender"] = student_info.gender;
            doc["class"] = student_info.class_name;
            doc["global_avg_grade"] = String((student_info.sprint_avg_grade + student_info.lap_run_avg_grade) / 2, 2);
            doc["sprint"]["avg_grade"] = String(student_info.sprint_avg_grade, 2);
            doc["sprint"]["avg_time"] = student_info.sprint_avg_time;
            doc["lap_run"]["avg_grade"] = String(student_info.lap_run_avg_grade, 2);
            doc["lap_run"]["avg_time"] = student_info.lap_run_avg_time;
            for (int i = 0; i < student_info.runs.size(); i++) {
                String index = String(i);
                doc["runs"][index]["type"] = student_info.runs[i].type;
                doc["runs"][index]["length"] = student_info.runs[i].length;
                doc["runs"][index]["grade"] = String(student_info.runs[i].grade, 2);
                doc["runs"][index]["time"] = student_info.runs[i].time;
                doc["runs"][index]["date"] = student_info.runs[i].date;
            }
        }
    } else {
        request_result = 400;
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}

void DTTSRestApi::deleteStudent(AsyncWebServerRequest *request) {
    int request_result = 200;

    if (request->hasParam("id")) {
        int student_id = request->getParam("id")->value().toInt();
        if (db->deleteStudent(student_id)) {
            request_result = 404;
        }
    } else {
        request_result = 400;
    }

    request->send(request_result, "application/json", "{}");
}

void DTTSRestApi::putStudent(AsyncWebServerRequest *request, String data) {
    int request_result = 201;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
        doc.clear();
    } else {
        if (doc.containsKey("name") && doc.containsKey("class_id") && doc.containsKey("gender")) {
            String name = doc["name"];
            int class_id = doc["class_id"];
            int gender = doc["gender"];
            doc.clear();

            int student_id = db->putStudent(name, gender, class_id);
            if (student_id == DB_FAILED) {
                request_result = 500;
            } else if (student_id == DB_CONFLICT) {
                request_result = 409;
            } else if (student_id == DB_NOT_FOUND) {
                request_result = 400;
            } else {
                doc["id"] = student_id;
            }
        } else {
            doc.clear();
            request_result = 400;
        }
    }
    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);   
}

void DTTSRestApi::patchStudent(AsyncWebServerRequest *request, String data) {
    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (doc.containsKey("name") && doc.containsKey("class_id")) {
            String name = doc["name"];
            int class_id = doc["class_id"];

            int result = db->patchStudent(name, class_id);
            if (result == DB_FAILED) {
                request_result = 500;
            } else if (result == DB_CONFLICT) {
                request_result = 409;
            } else if (result == DB_NOT_FOUND) {
                request_result = 400;
            }
        } else {
            request_result = 400;
        }
    }
    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", "{}");   
}