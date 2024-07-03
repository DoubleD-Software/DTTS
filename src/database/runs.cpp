#include <database.h>

/**
 * Fetches runs from the database by their date
 * @param date The date to fetch runs for (julian date)
 * @return A vector of RunInfo objects
*/
std::vector<RunInfo> Database::getRunsByDate(int date) {
    std::vector<RunInfo> runs;

    String sql = "SELECT * FROM runs WHERE date = " + String(date) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return runs;
    }
    
    while (sqlite3_step(run_stmt) == SQLITE_ROW) {
        RunInfo run;
        run.run_id = sqlite3_column_int(run_stmt, 0);
        run.type = sqlite3_column_int(run_stmt, 1);
        run.length = sqlite3_column_int(run_stmt, 7);
        sqlite3_stmt* teacher_stmt;
        String teacher_sql = "SELECT name FROM teachers WHERE id = " + String(sqlite3_column_int(run_stmt, 6)) + ";";
        if (sqlite3_prepare_v2(this->db, teacher_sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(teacher_stmt);
            break;
        }
        
        if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
            run.teacher = String((const char*) sqlite3_column_text(teacher_stmt, 0));
        } else {
            run.teacher = UNKNOWN_NAME;
        }

        sqlite3_finalize(teacher_stmt);
        sqlite3_stmt* class_stmt;
        String class_sql = "SELECT name FROM classes WHERE id = " + String(sqlite3_column_int(run_stmt, 3)) + ";";
        if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(class_stmt);
            break;
        }
        if (sqlite3_step(class_stmt) == SQLITE_ROW) {
            run.class_name = String((const char*) sqlite3_column_text(class_stmt, 0));
        } else {
            run.class_name = UNKNOWN_NAME;
        }
        sqlite3_finalize(class_stmt);
        String result_sql = "SELECT * FROM results WHERE run_id = " + String(sqlite3_column_int(run_stmt, 0)) + ";";
        sqlite3_stmt* result_stmt;
        if (sqlite3_prepare_v2(this->db, result_sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(result_stmt);
            break;
        }
        double total_grade = 0;
        double total_time = 0;
        int num_results = 0;
        while (sqlite3_step(result_stmt) == SQLITE_ROW) {
            total_grade += sqlite3_column_double(result_stmt, 6);
            total_time += sqlite3_column_int(result_stmt, 3);
            num_results++;
        }
        if (num_results == 0) {
            run.avg_grade = 0;
            run.avg_time = 0;
        } else {
            run.avg_grade = total_grade / num_results;
            run.avg_time = total_time / num_results;
        }
        sqlite3_finalize(result_stmt);
        runs.push_back(run);
    }

    sqlite3_finalize(run_stmt);
    return runs;
}

/**
 * Fetches runs from the database by their ID
 * @param run_id The ID of the run to fetch
 * @return A RunInfoSpecific object; if the run is not found, the type will be -1
*/
RunInfoSpecific Database::getRunInfo(int run_id) {
    RunInfoSpecific run;
    run.type = -1;

    String sql = "SELECT * FROM runs WHERE id = " + String(run_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return run;
    }
    int count = 0;
    while (sqlite3_step(run_stmt) == SQLITE_ROW && count < 1) {
        count++;
        run.type = sqlite3_column_int(run_stmt, 1);
        run.length = (int) sqlite3_column_int(run_stmt, 7);
        run.date = (int) sqlite3_column_int(run_stmt, 2);
        sqlite3_stmt* teacher_stmt;
        String teacher_sql = "SELECT name FROM teachers WHERE id = " + String((int) sqlite3_column_int(run_stmt, 6)) + ";";
        if (sqlite3_prepare_v2(this->db, teacher_sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(teacher_stmt);
            break;
        }
        if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
            run.teacher = String((const char*) sqlite3_column_text(teacher_stmt, 0));
        } else {
            run.teacher = UNKNOWN_NAME;
        }
        sqlite3_finalize(teacher_stmt);
        sqlite3_stmt* class_stmt;
        String class_sql = "SELECT name FROM classes WHERE id = " + String(sqlite3_column_int(run_stmt, 3)) + ";";
        if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(class_stmt);
            break;
        }
        if (sqlite3_step(class_stmt) == SQLITE_ROW) {
            run.class_name = String((const char*) sqlite3_column_text(class_stmt, 0));
        } else {
            run.class_name = UNKNOWN_NAME;
        }
        sqlite3_finalize(class_stmt);
        sqlite3_stmt* grading_key_m_stmt;
        String grading_key_m_sql = "SELECT name FROM grading_keys WHERE id = " + String(sqlite3_column_int(run_stmt, 4)) + ";";
        if (sqlite3_prepare_v2(this->db, grading_key_m_sql.c_str(), -1, &grading_key_m_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(grading_key_m_stmt);
            break;
        }
        if (sqlite3_step(grading_key_m_stmt) == SQLITE_ROW) {
            run.grading_key_m = String((const char*) sqlite3_column_text(grading_key_m_stmt, 0));
        } else {
            run.grading_key_m = UNKNOWN_NAME;
        }
        sqlite3_finalize(grading_key_m_stmt);
        sqlite3_stmt* grading_key_f_stmt;
        String grading_key_f_sql = "SELECT name FROM grading_keys WHERE id = " + String(sqlite3_column_int(run_stmt, 5)) + ";";
        if (sqlite3_prepare_v2(this->db, grading_key_f_sql.c_str(), -1, &grading_key_f_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(grading_key_f_stmt);
            break;
        }
        if (sqlite3_step(grading_key_f_stmt) == SQLITE_ROW) {
            run.grading_key_f = String((const char*) sqlite3_column_text(grading_key_f_stmt, 0));
        } else {
            run.grading_key_f = UNKNOWN_NAME;
        }
        sqlite3_finalize(grading_key_f_stmt);
        String result_sql = "SELECT * FROM results WHERE run_id = " + String(run_id) + ";";
        sqlite3_stmt* result_stmt;
        if (sqlite3_prepare_v2(this->db, result_sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(result_stmt);
            break;
        }
        double total_grade = 0;
        double total_time = 0;
        int num_results = 0;
        while (sqlite3_step(result_stmt) == SQLITE_ROW) {
            RunInfoStudent student;
            student.student_id = sqlite3_column_int(result_stmt, 2);
            sqlite3_stmt* student_stmt;
            String student_sql = "SELECT name, gender FROM students WHERE id = " + String(student.student_id) + ";";
            if (sqlite3_prepare_v2(this->db, student_sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
                DEBUG_SER_PRINT("Failed to prepare statement: ");
                DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                sqlite3_finalize(student_stmt);
                break;
            }
            if (sqlite3_step(student_stmt) == SQLITE_ROW) {
                student.name = String((const char*) sqlite3_column_text(student_stmt, 0));
            } else {
                student.name = UNKNOWN_NAME;
            }
            student.gender = sqlite3_column_int(student_stmt, 1);
            sqlite3_finalize(student_stmt);
            student.time = sqlite3_column_int(result_stmt, 3);
            student.grade = sqlite3_column_double(result_stmt, 6);
            total_grade += student.grade;
            total_time += student.time;
            num_results++;
            run.students.push_back(student);
        }
        if (num_results == 0) {
            run.avg_grade = 0;
            run.avg_time = 0;
        } else {
            run.avg_grade = total_grade / num_results;
            run.avg_time = total_time / num_results;
        }
        sqlite3_finalize(result_stmt);
    }

    sqlite3_finalize(run_stmt);
    return run;
}

/**
 * Fetches a specific student's information for a run
 * @param run_id The ID of the run
 * @param student_id The ID of the student
 * @return A RunInfoStudentLaps object; if the student is not found, the time will be -1
*/
RunInfoStudentLaps Database::getRunInfoStudent(int run_id, int student_id) {
    RunInfoStudentLaps student;
    student.time = -1;

    String sql = "SELECT * FROM results WHERE run_id = " + String(run_id) + " AND student_id = " + String(student_id) + ";";
    sqlite3_stmt* result_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(result_stmt);
        return student;
    }
    int count = 0;
    while (sqlite3_step(result_stmt) == SQLITE_ROW && count < 1) {
        int type = sqlite3_column_int(result_stmt, 4);
        if (type == RUN_TYPE_SPRINT) {
            student.time = -2;
            break;
        }

        count++;
        student.grade = sqlite3_column_double(result_stmt, 6);
        student.time = sqlite3_column_int(result_stmt, 3);
        sqlite3_stmt* student_stmt;
        String student_sql = "SELECT name FROM students WHERE id = " + String(student_id) + ";";
        if (sqlite3_prepare_v2(this->db, student_sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(student_stmt);
            break;
        }
        if (sqlite3_step(student_stmt) == SQLITE_ROW) {
            student.student_name = String((const char*) sqlite3_column_text(student_stmt, 0));
        } else {
            student.student_name = UNKNOWN_NAME;
        }
        sqlite3_finalize(student_stmt);
        String lap_sql = "SELECT * FROM laps WHERE result_id = " + String(sqlite3_column_int(result_stmt, 0)) + ";";
        sqlite3_stmt* lap_stmt;
        if (sqlite3_prepare_v2(this->db, lap_sql.c_str(), -1, &lap_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(lap_stmt);
            break;
        }
        while (sqlite3_step(lap_stmt) == SQLITE_ROW) {
            RunInfoLap lap;
            lap.length = sqlite3_column_int(lap_stmt, 3);
            student.length += lap.length;
            lap.time = sqlite3_column_int(lap_stmt, 2);
            student.laps.push_back(lap);
        }
        sqlite3_finalize(lap_stmt);
    }
    return student;
}

/**
 * Deletes a run from the database and all associated results and laps
 * @param run_id The ID of the run to delete
 * @return A status code defined in database.h
*/
int Database::deleteRun(int run_id) {
    String sql = "SELECT * FROM runs WHERE id = " + String(run_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(run_stmt) != SQLITE_ROW) {
        sqlite3_finalize(run_stmt);
        return DB_NOT_FOUND;
    }
    int run_type = sqlite3_column_int(run_stmt, 1);
    int class_id = sqlite3_column_int(run_stmt, 3);
    sqlite3_finalize(run_stmt);

    // Calculate the grade and time of the run and subtract it from the class averages
    double avg_grade = 0;
    int avg_time = 0;
    int num_results = 0;
    sql = "SELECT time, grade FROM results WHERE run_id = " + String(run_id) + ";";
    sqlite3_stmt* result_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(result_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(result_stmt) == SQLITE_ROW) {
        avg_grade += sqlite3_column_double(result_stmt, 1);
        avg_time += sqlite3_column_int(result_stmt, 0);
        num_results++;
    }
    sqlite3_finalize(result_stmt);

    if (num_results == 0) { // The run hasn't been graded yet
        return DB_SUCCESS;
    }

    avg_grade /= num_results;
    avg_time /= num_results;

    num_results = 0;
    sql = "SELECT id from runs WHERE class_id = " + String(class_id) + " AND type = " + String(run_type) + ";";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(result_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(result_stmt) == SQLITE_ROW) {
        num_results++;
    }
    sqlite3_finalize(result_stmt);

    sql = "SELECT sprint_avg_grade, sprint_avg_time, lap_run_avg_grade, lap_run_avg_time FROM classes WHERE id = " + String(class_id) + ";";
    sqlite3_stmt* class_stmt;

    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(class_stmt) == SQLITE_ROW) {
        double sprint_avg_grade = sqlite3_column_double(class_stmt, 0);
        int sprint_avg_time = sqlite3_column_int(class_stmt, 1);
        double lap_run_avg_grade = sqlite3_column_double(class_stmt, 2);
        int lap_run_avg_time = sqlite3_column_int(class_stmt, 3);

        if (num_results != 0) {
            if (run_type == RUN_TYPE_SPRINT) {
                sprint_avg_grade = (sprint_avg_grade * num_results - avg_grade) / num_results;
                sprint_avg_time = (sprint_avg_time * num_results - avg_time) / num_results;
            } else {
                lap_run_avg_grade = (lap_run_avg_grade * num_results - avg_grade) / num_results;
                lap_run_avg_time = (lap_run_avg_time * num_results - avg_time) / num_results;
            }
        } else {
            if (run_type == RUN_TYPE_SPRINT) {
                sprint_avg_grade = 0;
                sprint_avg_time = 0;
            } else {
                lap_run_avg_grade = 0;
                lap_run_avg_time = 0;
            }
        }

        sql = "UPDATE classes SET sprint_avg_grade = " + String(sprint_avg_grade) + ", sprint_avg_time = " + String(sprint_avg_time) + ", lap_run_avg_grade = " + String(lap_run_avg_grade) + ", lap_run_avg_time = " + String(lap_run_avg_time) + " WHERE id = " + String(class_id) + ";";
        if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to update class averages: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }

    sql = "DELETE FROM runs WHERE id = " + String(run_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete run: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sqlite3_finalize(class_stmt);
    return DB_SUCCESS;
}

/**
 * Creates a new run in the database
 * @param type The type of run
 * @param date The date of the run
 * @param class_id The ID of the class the run is for
 * @param grading_key_m_id The ID of the male grading key
 * @param grading_key_f_id The ID of the female grading key
 * @param teacher_id The ID of the teacher
 * @param length The length of the run in meters
 * @param laps The number of laps
 * @param participants A vector of student IDs participating in the run
 * @return The ID of the new run, or a status code defined in database.h
*/
int Database::putRun(int type, int date, int class_id, int grading_key_m_id, int grading_key_f_id, int teacher_id, int length, double laps, std::vector<int> participants) {
    String sql = "INSERT INTO runs (type, date, class_id, grading_key_m_id, grading_key_f_id, teacher_id, length, laps) VALUES (" + String(type) + ", " + String(date) + ", " + String(class_id) + ", " + String(grading_key_m_id) + ", " + String(grading_key_f_id) + ", " + String(teacher_id) + ", " + String(length) + ", " + String(laps, 2) + ");";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to insert run: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        if (sqlite3_errcode(this->db) == SQLITE_CONSTRAINT) {
            return DB_CONFLICT;
        } else {
            return DB_FAILED;
        }
    }
    int run_id = sqlite3_last_insert_rowid(this->db);
    for (int i = 0; i < participants.size(); i++) {
        String participant_sql = "INSERT INTO participants (run_id, student_id) VALUES (" + String(run_id) + ", " + String(participants[i]) + ");";
        if (sqlite3_exec(this->db, participant_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to insert participant: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }
    return run_id;
}

/**
 * Returns the run info for an active run that is in the state of being ran or the tag assignment state
 * @param run_id The ID of the run
 * @return A RunInfoActive object
*/
RunInfoActive Database::getRunInfoActive(int run_id) {
    RunInfoActive run_info;

    String sql = "SELECT * FROM runs WHERE id = " + String(run_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return run_info;
    }
    if (sqlite3_step(run_stmt) == SQLITE_ROW) {
        run_info.type = sqlite3_column_int(run_stmt, 1);
        run_info.date = sqlite3_column_int(run_stmt, 2);
        run_info.length = sqlite3_column_int(run_stmt, 7);
    } else {
        sqlite3_finalize(run_stmt);
        return run_info;
    }
    int class_id = sqlite3_column_int(run_stmt, 3);
    int grading_key_m_id = sqlite3_column_int(run_stmt, 4);
    int grading_key_f_id = sqlite3_column_int(run_stmt, 5);
    int teacher_id = sqlite3_column_int(run_stmt, 6);
    sqlite3_finalize(run_stmt);

    sql = "SELECT name FROM classes WHERE id = " + String(class_id) + ";";
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return run_info;
    }
    if (sqlite3_step(class_stmt) == SQLITE_ROW) {
        run_info.class_name = String((const char*) sqlite3_column_text(class_stmt, 0));
    } else {
        run_info.class_name = UNKNOWN_NAME;
    }
    sqlite3_finalize(class_stmt);

    sql = "SELECT * FROM grading_keys WHERE id = " + String(grading_key_m_id) + ";";
    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return run_info;
    }
    if (sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        run_info.grading_key_m = String((const char*) sqlite3_column_text(grading_key_stmt, 1));
    } else {
        run_info.grading_key_m = UNKNOWN_NAME;
    }
    sqlite3_finalize(grading_key_stmt);

    sql = "SELECT * FROM grading_keys WHERE id = " + String(grading_key_f_id) + ";";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return run_info;
    }
    if (sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        run_info.grading_key_f = String((const char*) sqlite3_column_text(grading_key_stmt, 1));
    } else {
        run_info.grading_key_f = UNKNOWN_NAME;
    }
    sqlite3_finalize(grading_key_stmt);

    sql = "SELECT * FROM teachers WHERE id = " + String(teacher_id) + ";";
    sqlite3_stmt* teacher_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return run_info;
    }
    if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
        run_info.teacher_name = String((const char*) sqlite3_column_text(teacher_stmt, 1));
    } else {
        run_info.teacher_name = UNKNOWN_NAME;
    }
    sqlite3_finalize(teacher_stmt);

    return run_info;
}

/**
 * Fetches the participants ids for a run
 * @param run_id The ID of the run
 * @return A vector of participant IDs
*/
std::vector<RunParticipant> Database::getRunParticipants(int run_id) {
    std::vector<RunParticipant> participants;

    String sql = "SELECT * FROM participants WHERE run_id = " + String(run_id) + ";";
    sqlite3_stmt* participant_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &participant_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(participant_stmt);
        return participants;
    }
    while (sqlite3_step(participant_stmt) == SQLITE_ROW) {
        RunParticipant participant;
        participant.student_id = sqlite3_column_int(participant_stmt, 1);
        sql = "SELECT name FROM students WHERE id = " + String(participant.student_id) + ";";
        sqlite3_stmt* student_stmt;
        if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(student_stmt);
            break;
        }
        if (sqlite3_step(student_stmt) == SQLITE_ROW) {
            participant.student_name = String((const char*) sqlite3_column_text(student_stmt, 0));
        } else {
            participant.student_name = UNKNOWN_NAME;
        }
        sqlite3_finalize(student_stmt);
        participants.push_back(participant);
    }
    sqlite3_finalize(participant_stmt);
    return participants;
}

/**
 * Inserts the results of a sprint into the database and updates the grades
 * @param run_id The ID of the run
 * @param finishers A vector of FinisherSprints objects
 * @return A status code defined in database.h
*/
int Database::insertSprintResults(int run_id, std::vector<FinisherSprint> finishers) {
    String run_sql = "SELECT * FROM runs WHERE id = " + String(run_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, run_sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(run_stmt) != SQLITE_ROW) {
        sqlite3_finalize(run_stmt);
        return DB_NOT_FOUND;
    }
    int grading_key_m_id = sqlite3_column_int(run_stmt, 4);
    int grading_key_f_id = sqlite3_column_int(run_stmt, 5);
    int class_id = sqlite3_column_int(run_stmt, 3);
    int date = sqlite3_column_int(run_stmt, 2);
    sqlite3_finalize(run_stmt);

    std::vector<GradingKeyGrade> grading_key_m;
    std::vector<GradingKeyGrade> grading_key_f;

    String grading_keys_grades_sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key_m_id);
    sqlite3_stmt* grading_keys_grades_stmt;
    if (sqlite3_prepare_v2(this->db, grading_keys_grades_sql.c_str(), -1, &grading_keys_grades_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_keys_grades_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(grading_keys_grades_stmt) == SQLITE_ROW) {
        GradingKeyGrade grade;
        grade.grade = sqlite3_column_double(grading_keys_grades_stmt, 2);
        grade.time = sqlite3_column_int(grading_keys_grades_stmt, 1);
        grading_key_m.push_back(grade);
    }
    sqlite3_finalize(grading_keys_grades_stmt);

    grading_keys_grades_sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key_f_id);
    if (sqlite3_prepare_v2(this->db, grading_keys_grades_sql.c_str(), -1, &grading_keys_grades_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_keys_grades_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(grading_keys_grades_stmt) == SQLITE_ROW) {
        GradingKeyGrade grade;
        grade.grade = sqlite3_column_double(grading_keys_grades_stmt, 2);
        grade.time = sqlite3_column_int(grading_keys_grades_stmt, 1);
        grading_key_f.push_back(grade);
    }
    sqlite3_finalize(grading_keys_grades_stmt);

    double avg_grade = 0;
    double avg_time = 0;

    for (int i = 0; i < finishers.size(); i++) {
        String student_sql = "SELECT gender FROM students WHERE id = " + String(finishers[i].student_id) + ";";
        sqlite3_stmt *student_stmt;
        if (sqlite3_prepare_v2(this->db, student_sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(student_stmt);
            return DB_FAILED;
        }
        if (sqlite3_step(student_stmt) != SQLITE_ROW) {
            sqlite3_finalize(student_stmt);
            return DB_NOT_FOUND;
        }
        int gender = sqlite3_column_int(student_stmt, 0);
        sqlite3_finalize(student_stmt);

        double grade = 6;

        if (gender == GENDER_TYPE_MALE) {
            int time_diff = INT32_MAX;
            for (int j = 0; j < grading_key_m.size(); j++) {
                if (((grading_key_m[j].time - finishers[i].time) >= 0) && ((grading_key_m[j].time - finishers[i].time) < time_diff)) {
                    time_diff = grading_key_m[j].time - finishers[i].time;
                    grade = grading_key_m[j].grade;
                }
            }
        } else {
            int time_diff = INT32_MAX;
            for (int j = 0; j < grading_key_f.size(); j++) {
                if (((grading_key_f[j].time - finishers[i].time) >= 0) && ((grading_key_f[j].time - finishers[i].time) < time_diff)) {
                    time_diff = grading_key_f[j].time - finishers[i].time;
                    grade = grading_key_f[j].grade;
                }
            }
        }

        String result_sql = "INSERT INTO results (type, run_id, student_id, time, grade, date) VALUES (0, " + String(run_id) + ", " + String(finishers[i].student_id) + ", " + String(finishers[i].time) + ", " + String(grade) + ", " + String(date) + ");";
        if (sqlite3_exec(this->db, result_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to insert result: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }

        avg_grade += grade;
        avg_time += finishers[i].time;
    }
    avg_grade /= finishers.size();
    avg_time /= finishers.size();

    run_sql = "SELECT * FROM runs WHERE type = 0 AND class_id = " + String(class_id);
    if (sqlite3_prepare_v2(this->db, run_sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return DB_FAILED;
    }
    int num_sprints = 0;
    while (sqlite3_step(run_stmt) == SQLITE_ROW) {
        num_sprints++;
    }

    sqlite3_finalize(run_stmt);

    String class_sql = "SELECT * FROM classes WHERE id = " + String(class_id);
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(class_stmt) != SQLITE_ROW) {
        sqlite3_finalize(class_stmt);
        return DB_NOT_FOUND;
    }
    double sprint_avg_grade = sqlite3_column_double(class_stmt, 2);
    int sprint_avg_time = sqlite3_column_int(class_stmt, 3);
    sqlite3_finalize(class_stmt);

    sprint_avg_grade = (sprint_avg_grade * (num_sprints - 1) + avg_grade) / num_sprints;
    sprint_avg_time = (sprint_avg_time * (num_sprints - 1) + avg_time) / num_sprints;

    class_sql = "UPDATE classes SET sprint_avg_grade = " + String(sprint_avg_grade) + ", sprint_avg_time = " + String(sprint_avg_time) + " WHERE id = " + String(class_id);
    if (sqlite3_exec(this->db, class_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to update class: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    return DB_SUCCESS;
}

/**
 * Gets the length and number of laps for a lap run
 * Used in the pre-calculation while waiting for a run start
 * @param run_id The ID of the run
 * @return A PreLapRunInfo
*/
PreLapRunInfo Database::getPreLapRunInfo(int run_id) {
    PreLapRunInfo run_info;

    String sql = "SELECT * FROM runs WHERE id = " + String(run_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return run_info;
    }
    if (sqlite3_step(run_stmt) == SQLITE_ROW) {
        run_info.length = sqlite3_column_int(run_stmt, 7);
        run_info.laps = sqlite3_column_double(run_stmt, 8);
    }
    sqlite3_finalize(run_stmt);
    return run_info;
}

/**
 * Inserts the results of a lap run into the database and updates the grades
 * @param run_id The ID of the run
 * @param finishers A vector of FinisherLapRun objects
 * @return A status code defined in database.h
*/
int Database::insertLapRunResults(int run_id, std::vector<FinisherLapRun> finishers) {
    String run_sql = "SELECT * FROM runs WHERE id = " + String(run_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, run_sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(run_stmt) != SQLITE_ROW) {
        sqlite3_finalize(run_stmt);
        return DB_NOT_FOUND;
    }
    int grading_key_m_id = sqlite3_column_int(run_stmt, 4);
    int grading_key_f_id = sqlite3_column_int(run_stmt, 5);
    int class_id = sqlite3_column_int(run_stmt, 3);
    int date = sqlite3_column_int(run_stmt, 2);
    sqlite3_finalize(run_stmt);

    std::vector<GradingKeyGrade> grading_key_m;
    std::vector<GradingKeyGrade> grading_key_f;

    String grading_keys_grades_sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key_m_id);
    sqlite3_stmt* grading_keys_grades_stmt;
    if (sqlite3_prepare_v2(this->db, grading_keys_grades_sql.c_str(), -1, &grading_keys_grades_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_keys_grades_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(grading_keys_grades_stmt) == SQLITE_ROW) {
        GradingKeyGrade grade;
        grade.grade = sqlite3_column_double(grading_keys_grades_stmt, 2);
        grade.time = sqlite3_column_int(grading_keys_grades_stmt, 1);
        grading_key_m.push_back(grade);
    }
    sqlite3_finalize(grading_keys_grades_stmt);

    grading_keys_grades_sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key_f_id);
    if (sqlite3_prepare_v2(this->db, grading_keys_grades_sql.c_str(), -1, &grading_keys_grades_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_keys_grades_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(grading_keys_grades_stmt) == SQLITE_ROW) {
        GradingKeyGrade grade;
        grade.grade = sqlite3_column_double(grading_keys_grades_stmt, 2);
        grade.time = sqlite3_column_int(grading_keys_grades_stmt, 1);
        grading_key_f.push_back(grade);
    }
    sqlite3_finalize(grading_keys_grades_stmt);

    double avg_grade = 0;
    double avg_time = 0;
    
    for (int i = 0; i < finishers.size(); i++) {
        String student_sql = "SELECT gender FROM students WHERE id = " + String(finishers[i].student_id) + ";";
        sqlite3_stmt *student_stmt;
        if (sqlite3_prepare_v2(this->db, student_sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(student_stmt);
            return DB_FAILED;
        }
        if (sqlite3_step(student_stmt) != SQLITE_ROW) {
            sqlite3_finalize(student_stmt);
            return DB_NOT_FOUND;
        }
        int gender = sqlite3_column_int(student_stmt, 0);
        sqlite3_finalize(student_stmt);

        double grade = 6;

        if (gender == GENDER_TYPE_MALE) {
            int time_diff = INT32_MAX;
            for (int j = 0; j < grading_key_m.size(); j++) {
                if (((grading_key_m[j].time - finishers[i].total_time) >= 0) && ((grading_key_m[j].time - finishers[i].total_time) < time_diff)) {
                    time_diff = grading_key_m[j].time - finishers[i].total_time;
                    grade = grading_key_m[j].grade;
                }
            }
        } else {
            int time_diff = INT32_MAX;
            for (int j = 0; j < grading_key_f.size(); j++) {
                if (((grading_key_f[j].time - finishers[i].total_time) >= 0) && ((grading_key_f[j].time - finishers[i].total_time) < time_diff)) {
                    time_diff = grading_key_f[j].time - finishers[i].total_time;
                    grade = grading_key_f[j].grade;
                }
            }
        }

        String result_sql = "INSERT INTO results (type, run_id, student_id, time, grade, date) VALUES (1, " + String(run_id) + ", " + String(finishers[i].student_id) + ", " + String(finishers[i].total_time) + ", " + String(grade) + ", " + String(date) + ");";
        if (sqlite3_exec(this->db, result_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to insert result: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }

        int result_id = sqlite3_last_insert_rowid(this->db);

        avg_grade += grade;
        avg_time += finishers[i].total_time;

        for (int j = 0; j < finishers[i].laps.size(); j++) {
            String lap_sql = "INSERT INTO laps (lap_num, result_id, time, length) VALUES (" + String(i + 1) + ", " + String(result_id) + ", " + String(finishers[i].laps[j].time) + ", " + String(finishers[i].laps[j].length) + ");";
            if (sqlite3_exec(this->db, lap_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
                DEBUG_SER_PRINT("Failed to insert lap: ");
                DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                return DB_FAILED;
            }
        }
    }
    avg_grade /= finishers.size();
    avg_time /= finishers.size();

    run_sql = "SELECT * FROM runs WHERE type = 1 AND class_id = " + String(class_id);
    if (sqlite3_prepare_v2(this->db, run_sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return DB_FAILED;
    }
    int num_lap_runs = 0;
    while (sqlite3_step(run_stmt) == SQLITE_ROW) {
        num_lap_runs++;
    }

    sqlite3_finalize(run_stmt);

    String class_sql = "SELECT * FROM classes WHERE id = " + String(class_id);
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(class_stmt) != SQLITE_ROW) {
        sqlite3_finalize(class_stmt);
        return DB_NOT_FOUND;
    }
    double lap_run_avg_grade = sqlite3_column_double(class_stmt, 4);
    int lap_run_avg_time = sqlite3_column_int(class_stmt, 5);
    sqlite3_finalize(class_stmt);

    lap_run_avg_grade = (lap_run_avg_grade * (num_lap_runs - 1) + avg_grade) / num_lap_runs;
    lap_run_avg_time = (lap_run_avg_time * (num_lap_runs - 1) + avg_time) / num_lap_runs;

    class_sql = "UPDATE classes SET lap_run_avg_grade = " + String(lap_run_avg_grade) + ", lap_run_avg_time = " + String(lap_run_avg_time) + " WHERE id = " + String(class_id);
    if (sqlite3_exec(this->db, class_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to update class: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    return DB_SUCCESS;
}