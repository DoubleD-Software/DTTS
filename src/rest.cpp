#include <rest.h>

/**
 * This file contains all endpoints for the REST API.
 * A detailed description of each endpoint can be found at https://git.double-d.software/DoubleD-Software/DTTS/wiki/Routes


 * Constructor for the DTTSRestApi class.
 * @param db Pointer to the database object.
*/
DTTSRestApi::DTTSRestApi(Database *db, RunHandler *run_handler) {
    this->db = db;
    this->run_handler = run_handler;
}

/**
 * GET request handler for the /api/runs endpoint. This function returns information about runs.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::getRun(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    JsonDocument doc;
    int request_result = 200;

    if (request->hasParam("date")) {
        int date = request->getParam("date")->value().toInt();
        std::vector<RunInfo> runs = db->getRunsByDate(date);
        if (runs.size() == 0) {
            request_result = 404;
        } else {
            for (int i = 0; i < runs.size(); i++) {
                if (runs[i].avg_grade == 0 || runs[i].avg_time == 0) continue;

                String index = String(runs[i].run_id);
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
                String index = String(i + 1);
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
            doc["grading_key_male"] = run_info.grading_key_m;
            doc["grading_key_female"] = run_info.grading_key_f;
            doc["avg_grade"] = String(run_info.avg_grade, 2);
            doc["avg_time"] = run_info.avg_time;
            for (int i = 0; i < run_info.students.size(); i++) {
                String index = String(run_info.students[i].student_id);
                doc["students"][index]["name"] = run_info.students[i].name;
                doc["students"][index]["grade"] = String(run_info.students[i].grade, 2);
                doc["students"][index]["time"] = run_info.students[i].time;
                doc["students"][index]["gender"] = run_info.students[i].gender;
            }
        }
    } else {
        request_result = 400;
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}

/**
 * DELETE request handler for the /api/runs endpoint. This function deletes a run from the database.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::deleteRun(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

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

/**
 * PUT request handler for the /api/runs endpoint. This function adds a run to the database.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::putRun(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (doc.containsKey("type") && doc.containsKey("length") && doc.containsKey("date") && doc.containsKey("class_id") && doc.containsKey("grading_key_male_id") && doc.containsKey("grading_key_female_id") && doc.containsKey("laps") && doc.containsKey("participants")) {
            std::vector<int> participants;
            for (int i = 0; i < doc["participants"].size(); i++) {
                participants.push_back(doc["participants"][i]);
            }
            int type = doc["type"];
            int length = doc["length"];
            int date = doc["date"];
            int class_id = doc["class_id"];
            int teacher_id = logged_in_user_id;
            int grading_key_m_id = doc["grading_key_male_id"];
            int grading_key_f_id = doc["grading_key_female_id"];
            String laps_s = doc["laps"];
            double laps = laps_s.toDouble();

            int run_id = db->putRun(type, date, class_id, grading_key_m_id, grading_key_f_id, teacher_id, length, laps, participants);

            if (run_id != DB_FAILED) {
                run_handler->setActiveRun(run_id, type);
            } else {
                request_result = 500;
            }
        } else {
            request_result = 400;
        }
    }

    request->send(request_result, "application/json", "{}");
}

/**
 * GET request handler for the /api/students endpoint. This function returns information about students.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::getStudent(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

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
            doc["global_avg_grade"] = String(student_info.global_avg_grade, 2);
            doc["sprint"]["avg_grade"] = String(student_info.sprint_avg_grade, 2);
            doc["sprint"]["avg_time"] = student_info.sprint_avg_time;
            doc["lap_run"]["avg_grade"] = String(student_info.lap_run_avg_grade, 2);
            doc["lap_run"]["avg_time"] = student_info.lap_run_avg_time;
            for (int i = 0; i < student_info.runs.size(); i++) {
                String index = String(student_info.runs[i].run_id);
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

/**
 * DELETE request handler for the /api/students endpoint. This function deletes a student from the database.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::deleteStudent(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

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

/**
 * PUT request handler for the /api/students endpoint. This function adds a student to the database.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::putStudent(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

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

/**
 * PATCH request handler for the /api/students endpoint. This function updates a student in the database.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::patchStudent(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (request->hasArg("id") && (doc.containsKey("name") || doc.containsKey("class_id"))) {
            int result;
            int id = request->getParam("id")->value().toInt();
            String name = "";
            int class_id = -1;
            if (doc.containsKey("name")) {
                name = doc["name"].as<String>();
            }
            if (doc.containsKey("class_id")) {
                class_id = doc["class_id"];
            }

            result = db->patchStudent(id, name, class_id);
            if (result == DB_FAILED) {
                request_result = 500;
            } else if (result == DB_CONFLICT) {
                request_result = 409;
            } else if (result == DB_NOT_FOUND) {
                request_result = 404;
            }
        } else {
            request_result = 400;
        }
    }
    request->send(request_result, "application/json", "{}");   
}

/**
 * GET request handler for the /api/gradingkeys endpoint. This function returns information about grading keys.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::getGradingKeys(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;

    if (request->hasParam("id")) {
        int grading_key_id = request->getParam("id")->value().toInt();
        GradingKey grading_key = db->getGradingKey(grading_key_id);
        if (grading_key.type == -1) {
            request_result = 404;
        } else {
            doc["name"] = grading_key.name;
            doc["type"] = grading_key.type;
            doc["length"] = grading_key.length;
            doc["gender"] = grading_key.gender;
            for (int i = 0; i < grading_key.grades.size(); i++) {
                String index = String(grading_key.grades[i].grade);
                doc["grades"][index] = grading_key.grades[i].time;
            }
        }
    } else if (request->hasParam("type") && request->hasParam("length")) {
        int type = request->getParam("type")->value().toInt();
        int length = request->getParam("length")->value().toInt();
        GradingKeyMap grading_key_map = db->getGradingKeyMap(type, length);
        for (int i = 0; i < grading_key_map.males.size(); i++) {
            String index = String(grading_key_map.males[i].id);
            doc["male"][index] = grading_key_map.males[i].name;
        }
        for (int i = 0; i < grading_key_map.females.size(); i++) {
            String index = String(grading_key_map.females[i].id);
            doc["female"][index] = grading_key_map.females[i].name;
        }
    } else {
        std::vector<GradingKeySimple> grading_keys = db->getGradingKeys();
        for (int i = 0; i < grading_keys.size(); i++) {
            String index = String(grading_keys[i].id);
            doc[index]["name"] = grading_keys[i].name;
            doc[index]["type"] = grading_keys[i].type;
            doc[index]["min_time"] = grading_keys[i].min_time;
            doc[index]["length"] = grading_keys[i].length;
            doc[index]["gender"] = grading_keys[i].gender;
        }
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}

/**
 * DELETE request handler for the /api/gradingkeys endpoint. This function deletes a grading key from the database.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::deleteGradingKey(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;

    if (request->hasParam("id")) {
        int grading_key_id = request->getParam("id")->value().toInt();
        if (db->deleteGradingKey(grading_key_id)) {
            request_result = 404;
        }
    } else {
        request_result = 400;
    }

    request->send(request_result, "application/json", "{}");
}

/**
 * PUT request handler for the /api/gradingkeys endpoint. This function adds a grading key to the database.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::putGradingKey(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 201;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
        doc.clear();
    } else {
        if (doc.containsKey("name") && doc.containsKey("type") && doc.containsKey("length") && doc.containsKey("gender") && doc.containsKey("grades")) {
            String name = doc["name"];
            int type = doc["type"];
            int length = doc["length"];
            int gender = doc["gender"];
            std::vector<GradingKeyGrade> grades;
            
            JsonObject grades_obj = doc["grades"];
            for (JsonPair grade : grades_obj) {
                GradingKeyGrade grading_key_grade;
                String grade_key = grade.key().c_str();
                grading_key_grade.grade = grade_key.toDouble();
                grading_key_grade.time = grade.value().as<int>();
                grades.push_back(grading_key_grade);
            }
            doc.clear();

            if (grades.size() == 0) {
                request_result = 400;
            } else {
                int grading_key_id = db->putGradingKey(name, type, length, gender, grades);
                if (grading_key_id == DB_FAILED) {
                    request_result = 500;
                } else if (grading_key_id == DB_CONFLICT) {
                    request_result = 409;
                } else if (grading_key_id == DB_NOT_FOUND) {
                    request_result = 400;
                } else {
                    doc["id"] = grading_key_id;
                }
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

/**
 * PATCH request handler for the /api/gradingkeys endpoint. This function updates a grading key in the database.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::patchGradingKey(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (request->hasArg("id") && (doc.containsKey("name") || doc.containsKey("length"))) {
            int result;
            int id = request->getParam("id")->value().toInt();
            String name = "";
            int length = -1;
            if (doc.containsKey("name")) {
                name = doc["name"].as<String>();
            }
            if (doc.containsKey("length")) {
                length = doc["length"];
            }

            std::vector<GradingKeyGrade> grades;
            if (doc.containsKey("grades")) {
                JsonObject grades_obj = doc["grades"];
                for (JsonPair grade : grades_obj) {
                    GradingKeyGrade grading_key_grade;
                    String grade_key = grade.key().c_str();
                    grading_key_grade.grade = grade_key.toDouble();
                    grading_key_grade.time = grade.value().as<int>();
                    grades.push_back(grading_key_grade);
                }
            }

            result = db->patchGradingKey(id, name, length, grades);
            if (result == DB_FAILED) {
                request_result = 500;
            } else if (result == DB_CONFLICT) {
                request_result = 409;
            } else if (result == DB_NOT_FOUND) {
                request_result = 404;
            }
        } else {
            request_result = 400;
        }
    }
    request->send(request_result, "application/json", "{}");
}

/**
 * GET request handler for the /api/classes endpoint. This function returns information about classes.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::getClasses(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;

    if (request->hasParam("namesOnly")) {
        int names_only = request->getParam("namesOnly")->value().toInt();
        if (names_only != 1) {
            request_result = 400;
        } else {
            std::vector<ClassName> classes = db->getClassNames();
            for (int i = 0; i < classes.size(); i++) {
                String index = String(classes[i].class_id);
                doc[index] = classes[i].name;
            }
        }
    } else if (request->hasParam("id") && request->hasParam("studentsOnly")) {
        int students_only = request->getParam("studentsOnly")->value().toInt();
        if (students_only != 1) {
            request_result = 400;
        } else {
            int class_id = request->getParam("id")->value().toInt();
            std::vector<ClassStudent> students = db->getClassStudents(class_id);
            for (int i = 0; i < students.size(); i++) {
                String index = String(students[i].student_id);
                doc[index] = students[i].name;
            }
        }
    } else if (request->hasParam("id")) {
        int class_id = request->getParam("id")->value().toInt();
        ClassInfo class_info = db->getClassInfo(class_id);
        if (class_info.name == "") {
            request_result = 404;
        } else {
            doc["name"] = class_info.name;
            doc["global_avg_grade"] = String(class_info.global_avg_grade, 2);
            doc["sprint"]["avg_grade"] = String(class_info.sprint_avg_grade, 2);
            doc["sprint"]["avg_time"] = class_info.sprint_avg_time;
            doc["lap_run"]["avg_grade"] = String(class_info.lap_run_avg_grade, 2);
            doc["lap_run"]["avg_time"] = class_info.lap_run_avg_time;
            for (int i = 0; i < class_info.runs.size(); i++) {
                if (class_info.runs[i].avg_grade == 0 || class_info.runs[i].avg_grade == 0) continue;

                String index = String(class_info.runs[i].run_id);
                doc["runs"][index]["type"] = class_info.runs[i].type;
                doc["runs"][index]["length"] = class_info.runs[i].length;
                doc["runs"][index]["avg_grade"] = String(class_info.runs[i].avg_grade, 2);
                doc["runs"][index]["avg_time"] = class_info.runs[i].avg_time;
                doc["runs"][index]["date"] = class_info.runs[i].date;
            }
            for (int i = 0; i < class_info.students.size(); i++) {
                String index = String(class_info.students[i].student_id);
                doc["students"][index]["name"] = class_info.students[i].name;
                doc["students"][index]["gender"] = class_info.students[i].gender;
                doc["students"][index]["avg_grade"] = String(class_info.students[i].avg_grade, 2);
            }
        }
    } else {
        std::vector<ClassInfoSimple> classes = db->getClasses();
        for (int i = 0; i < classes.size(); i++) {
            String index = String(classes[i].class_id);
            doc[index]["name"] = classes[i].name;
            doc[index]["size"] = classes[i].size;
            doc[index]["sprint"]["avg_grade"] = String(classes[i].sprint_avg_grade, 2);
            doc[index]["sprint"]["avg_time"] = classes[i].sprint_avg_time;
            doc[index]["lap_run"]["avg_grade"] = String(classes[i].lap_run_avg_grade, 2);
            doc[index]["lap_run"]["avg_time"] = classes[i].lap_run_avg_time;
        }
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}

/**
 * DELETE request handler for the /api/classes endpoint. This function deletes a class from the database.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::deleteClass(AsyncWebServerRequest *request) { 
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;

    if (request->hasParam("id")) {
        int class_id = request->getParam("id")->value().toInt();
        if (db->deleteClass(class_id)) {
            request_result = 404;
        }
    } else {
        request_result = 400;
    }

    request->send(request_result, "application/json", "{}");
}

/**
 * PUT request handler for the /api/classes endpoint. This function adds a class to the database.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::putClass(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 201;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
        doc.clear();
    } else {
        if (doc.containsKey("name")) {
            String name = doc["name"];
            doc.clear();

            int class_id = db->putClass(name);
            if (class_id == DB_FAILED) {
                request_result = 500;
            } else if (class_id == DB_CONFLICT) {
                request_result = 409;
            } else if (class_id == DB_NOT_FOUND) {
                request_result = 400;
            } else {
                doc["id"] = class_id;
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

/**
 * PATCH request handler for the /api/classes endpoint. This function updates a class in the database.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::patchClass(AsyncWebServerRequest *request, String data) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (request->hasArg("id") && doc.containsKey("name")) {
            int result;
            int id = request->getParam("id")->value().toInt();
            String name = doc["name"].as<String>();

            result = db->patchClass(id, name);
            if (result == DB_FAILED) {
                request_result = 500;
            } else if (result == DB_CONFLICT) {
                request_result = 409;
            } else if (result == DB_NOT_FOUND) {
                request_result = 404;
            }
        } else {
            request_result = 400;
        }
    }
    request->send(request_result, "application/json", "{}");  
}

/**
 * GET request handler for the /api/teachers endpoint. This function returns information about teachers.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::getTeachers(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;

    if (request->hasParam("id")) {
        int teacher_id = request->getParam("id")->value().toInt();
        Teacher teacher = db->getTeacher(teacher_id);
        if (teacher.name == "") {
            request_result = 404;
        } else {
            doc["name"] = teacher.name;
            doc["username"] = teacher.username;
        }
    } else {
        std::vector<TeacherSimple> teachers = db->getTeachers();
        for (int i = 0; i < teachers.size(); i++) {
            String index = String(teachers[i].teacher_id);
            doc[index]["name"] = teachers[i].name;
        }
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(request_result, "application/json", json_output);
}

/**
 * DELETE request handler for the /api/teachers endpoint. This function deletes a teacher from the database and requires admin access.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::deleteTeacher(AsyncWebServerRequest *request) {
    if (access_level != ACCESS_ADMIN) {
        request->send(403, "application/json", "{}");
        return;
    } 
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;

    if (request->hasParam("id")) {
        int teacher_id = request->getParam("id")->value().toInt();
        if (db->deleteTeacher(teacher_id)) {
            request_result = 404;
        }
    } else {
        request_result = 400;
    }

    request->send(request_result, "application/json", "{}");
}

/**
 * PUT request handler for the /api/teachers endpoint. This function adds a teacher to the database and requires admin access.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::putTeacher(AsyncWebServerRequest *request, String data) {
    if (access_level != ACCESS_ADMIN) {
        request->send(403, "application/json", "{}");
        return;
    } 
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 201;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
        doc.clear();
    } else {
        if (doc.containsKey("name") && doc.containsKey("username") && doc.containsKey("password")) {
            String name = doc["name"];
            String username = doc["username"];
            String password = doc["password"];
            doc.clear();

            int teacher_id = db->putTeacher(name, username, password);
            if (teacher_id == DB_FAILED) {
                request_result = 500;
            } else if (teacher_id == DB_CONFLICT) {
                request_result = 409;
            } else if (teacher_id == DB_NOT_FOUND) {
                request_result = 400;
            } else {
                doc["id"] = teacher_id;
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

/**
 * PATCH request handler for the /api/teachers endpoint. This function updates a teacher in the database and requires admin access.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::patchTeacher(AsyncWebServerRequest *request, String data) {
    if (access_level != ACCESS_ADMIN) {
        request->send(403, "application/json", "{}");
        return;
    } 
    if (checkAuth(request) == ACCESS_DENIED) return;

    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (request->hasArg("id") && (doc.containsKey("name") || doc.containsKey("username") || doc.containsKey("password"))) {
            int result;
            int id = request->getParam("id")->value().toInt();
            String name = "";
            String username = "";
            String password = "";
            if (doc.containsKey("name")) {
                name = doc["name"].as<String>();
            }
            if (doc.containsKey("username")) {
                username = doc["username"].as<String>();
            }
            if (doc.containsKey("password")) {
                password = doc["password"].as<String>();
            }

            result = db->patchTeacher(id, name, username, password);
            if (result == DB_FAILED) {
                request_result = 500;
            } else if (result == DB_CONFLICT) {
                request_result = 409;
            } else if (result == DB_NOT_FOUND) {
                request_result = 404;
            }
        } else {
            request_result = 400;
        }
    }
    request->send(request_result, "application/json", "{}");  
}

/**
 * GET request handler for the /api/whoami endpoint. This function returns if the user is an admin or not.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::whoAmI(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    JsonDocument doc;

    if (access_level == ACCESS_ADMIN) {
        doc["admin"] = true;
    } else {
        doc["admin"] = false;
    }

    String json_output;
    serializeJson(doc, json_output);
    request->send(200, "application/json", json_output);
}

/**
 * POST request handler for the /api/authenticate endpoint. This function authenticates a user and returns a session ID in a cookie.
 * @param request Pointer to the request object.
 * @param data String containing the JSON data.
*/
void DTTSRestApi::authenticate(AsyncWebServerRequest *request, String data) {
    int request_result = 200;
    JsonDocument doc;
    deserializeJson(doc, data);
    if (doc.isNull()) {
        request_result = 400;
    } else {
        if (doc.containsKey("username") && doc.containsKey("password") && doc.containsKey("date")) {
            current_date = doc["date"];
            String username = doc["username"];
            String password = doc["password"];

            UserAuth auth = db->checkPassword(username, password);
            logged_in_user_id = auth.user_id;
            access_level = auth.access_level;

            if (access_level >= 0) {
                user_logged_in = true;
                this->current_session_id = random(0xFFFF, 0x7FFFFFFF);
                
                AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{}");
                response->addHeader("Set-Cookie", "SESSIONID=" + String(this->current_session_id));
                request->send(response);
                return;
            } else {
                request_result = 401;
            }
        } else {
            request_result = 400;
        }
    }
    request->send(request_result, "application/json", "{}");
}

/**
 * GET request handler for th /api/active endpoint. This function returns information about the active run.
 * @param request Pointer to the request object.
*/
void DTTSRestApi::getActive(AsyncWebServerRequest *request) {
    if (checkAuth(request) == ACCESS_DENIED) return;

    JsonDocument doc;
    int request_result = 200;

    int run_id = run_handler->getActiveRunId();

    if (run_id == RUN_NOT_ACTIVE) {
        request_result = 404;
    } else if (run_id != RUN_ACTIVE_TAG) {
        RunInfoActive run_info = db->getRunInfoActive(run_id);
        doc["type"] = run_info.type;
        doc["length"] = run_info.length;
        doc["date"] = run_info.date;
        doc["class"] = run_info.class_name;
        doc["grading_key_male"] = run_info.grading_key_m;
        doc["grading_key_female"] = run_info.grading_key_f;
        doc["teacher"] = run_info.teacher_name;
    }

    String json_output;
    serializeJson(doc, json_output);
    if (json_output == "null") json_output = "";
    request->send(request_result, "application/json", json_output);
}

/**
 * This function checks the session ID of a request and returns if the user is logged in or not.
 * @param request Pointer to the request object.
*/
int DTTSRestApi::checkAuth(AsyncWebServerRequest *request) {
    if (user_logged_in) {
        if (request->hasHeader("Cookie")) {
            int cookie_index = request->header("Cookie").indexOf("SESSIONID=");
            if (cookie_index != -1) {
                String session_id = request->header("Cookie").substring(cookie_index + 10);
                if (session_id.toInt() == this->current_session_id) {
                    return ACCESS_GRANTED;
                }
            }
        }
    }
    request->send(401, "application/json", "{}");
    return ACCESS_DENIED;
}