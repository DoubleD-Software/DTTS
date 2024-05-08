#include <database.h>

const SqlTable Database::TABLES[] = {
    SqlTable("teachers", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_TEXT),
        SqlColumn("username", SQL_TYPE_TEXT),
        SqlColumn("password", SQL_TYPE_TEXT),
        SqlColumn("administrator", SQL_TYPE_INT)
    }),
    SqlTable("classes", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_TEXT),
        SqlColumn("sprint_avg_grade", SQL_TYPE_REAL),
        SqlColumn("sprint_avg_time", SQL_TYPE_INT),
        SqlColumn("lap_run_avg_grade", SQL_TYPE_REAL),
        SqlColumn("lap_run_avg_time", SQL_TYPE_INT)
    }),
    SqlTable("grading_keys", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_TEXT),
        SqlColumn("type", SQL_TYPE_INT),
        SqlColumn("length", SQL_TYPE_INT),
        SqlColumn("gender", SQL_TYPE_INT)
    }),
    SqlTable("grading_keys_grades", {
        SqlColumn("grading_key_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "grading_keys", "id"),
        SqlColumn("time", SQL_TYPE_INT),
        SqlColumn("grade", SQL_TYPE_REAL)
    }),
    SqlTable("students", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_TEXT),
        SqlColumn("gender", SQL_TYPE_INT),
        SqlColumn("class_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "classes", "id")
    }),
    SqlTable("runs", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("type", SQL_TYPE_INT),
        SqlColumn("date", SQL_TYPE_INT),
        SqlColumn("class_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "classes", "id"),
        SqlColumn("grading_key_m_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "grading_keys", "id"),
        SqlColumn("grading_key_f_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "grading_keys", "id"),
        SqlColumn("teacher_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "teachers", "id"),
        SqlColumn("length", SQL_TYPE_INT),
        SqlColumn("laps", SQL_TYPE_REAL),
    }),
    SqlTable("results", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("run_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "runs", "id"),
        SqlColumn("student_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "students", "id"),
        SqlColumn("time", SQL_TYPE_INT),
        SqlColumn("type", SQL_TYPE_INT),
        SqlColumn("date", SQL_TYPE_INT),
        SqlColumn("grade", SQL_TYPE_REAL)
    }),
    SqlTable("laps", {
        SqlColumn("lap_num", SQL_TYPE_INT),
        SqlColumn("result_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "results", "id"),
        SqlColumn("time", SQL_TYPE_INT),
        SqlColumn("length", SQL_TYPE_INT)
    }),
    SqlTable("participants", {
        SqlColumn("run_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "runs", "id"),
        SqlColumn("student_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "students", "id")
    })
};

Database::Database(String db_path) {
    this->db_path = db_path;
}

void Database::open() {
    if (sqlite3_open(this->db_path.c_str(), &this->db) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to open database: ");
        DEBUG_SER_PRINTLN(this->db_path);
        DEBUG_SER_PRINTLN("Creating database.");
        File db_file = SD.open(this->db_path, FILE_WRITE, true);
        if (!db_file) {
            DEBUG_SER_PRINT("Failed to create database file: ");
            DEBUG_SER_PRINTLN(this->db_path);
            sysHalt(DB_CREATE_FAILED, "Failed to create database file.");
        }
        db_file.flush();
        db_file.close();
        if (sqlite3_open(this->db_path.c_str(), &this->db) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to open database: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sysHalt(DB_OPEN_FAILED, "Failed to open database.");
        }
    }
    createTables();
}

void Database::close() {
    sqlite3_close(this->db);
}

void Database::createTables() {
    DEBUG_SER_PRINTLN("Enabling foreign key support (PRAGMA foreign_keys = ON;)");
    if (sqlite3_exec(this->db, "PRAGMA foreign_keys = ON;", 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to enable foreign key support: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sysHalt(DB_CREATE_FAILED, "Failed to enable foreign key support.");
    }

    for (int i = 0; i < sizeof(TABLES) / sizeof(SqlTable); i++) {
        SqlTable table = TABLES[i];
        String sql = "CREATE TABLE IF NOT EXISTS " + table.name + " (";
        String foreign_key_statements = "";
        for (int j = 0; j < table.columns.size(); j++) {
            SqlColumn column = table.columns[j];
            sql += column.name;
            switch (column.type) {
                case SQL_TYPE_INT:
                    sql += " INTEGER";
                    break;
                case SQL_TYPE_REAL:
                    sql += " DOUBLE";
                    break;
                case SQL_TYPE_TEXT:
                    sql += " TEXT";
                    break;
            }
            if (column.flags & SQL_TYPE_PRIMARY_KEY) {
                sql += " PRIMARY KEY";
            }
            if (column.flags & SQL_TYPE_AUTO_INCREMENT) {
                sql += " AUTOINCREMENT";
            }
            if (column.flags & SQL_TYPE_FOREIGN_KEY) {
                foreign_key_statements += ", FOREIGN KEY(" + String(column.name) + ") REFERENCES " + column.foreign_table + "(" + column.foreign_column + ") ON DELETE CASCADE";
            }
            if (j < table.columns.size() - 1) {
                sql += ", ";
            }
        }
        sql += foreign_key_statements;
        sql += ");";
        DEBUG_SER_PRINTLN(sql);
        if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to create table: ");
            DEBUG_SER_PRINTLN(table.name);
            DEBUG_SER_PRINT("Error: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sysHalt(DB_CREATE_FAILED, "Failed to create table.");
        }
    }
}

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
        float total_grade = 0;
        float total_time = 0;
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
        float total_grade = 0;
        float total_time = 0;
        int num_results = 0;
        while (sqlite3_step(result_stmt) == SQLITE_ROW) {
            RunInfoStudent student;
            student.student_id = sqlite3_column_int(result_stmt, 2);
            sqlite3_stmt* student_stmt;
            String student_sql = "SELECT name FROM students WHERE id = " + String(student.student_id) + ";";
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
            lap.lap_number = sqlite3_column_int(lap_stmt, 0);
            lap.length = sqlite3_column_int(lap_stmt, 3);
            student.length += lap.length;
            lap.time = sqlite3_column_int(lap_stmt, 2);
            student.laps.push_back(lap);
        }
        sqlite3_finalize(lap_stmt);
    }
    return student;
}

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
    sqlite3_finalize(run_stmt);

    sql = "DELETE FROM runs WHERE id = " + String(run_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete run: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    return DB_SUCCESS;
}

int Database::putRun(int type, int date, int class_id, int grading_key_m_id, int grading_key_f_id, int teacher_id, int length, float laps, std::vector<int> participants) {
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

StudentInfo Database::getStudentInfo(int student_id) {
    StudentInfo student;

    String sql = "SELECT * FROM students WHERE id = " + String(student_id) + ";";
    sqlite3_stmt* student_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return student;
    }
    int count = 0;
    while (sqlite3_step(student_stmt) == SQLITE_ROW && count < 1) {
        count++;
        student.student_name = String((const char*) sqlite3_column_text(student_stmt, 1));
        student.gender = sqlite3_column_int(student_stmt, 1);
        sqlite3_stmt* class_stmt;
        String class_sql = "SELECT name FROM classes WHERE id = " + String(sqlite3_column_int(student_stmt, 3)) + ";";
        if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(class_stmt);
            break;
        }
        if (sqlite3_step(class_stmt) == SQLITE_ROW) {
            student.class_name = String((const char*) sqlite3_column_text(class_stmt, 0));
        } else {
            student.class_name = UNKNOWN_NAME;
        }
        sqlite3_finalize(class_stmt);

        String result_sql = "SELECT * FROM results WHERE student_id = " + String(student_id) + ";";
        sqlite3_stmt* result_stmt;
        if (sqlite3_prepare_v2(this->db, result_sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(result_stmt);
            break;
        }
        float sprint_total_grade = 0;
        float sprint_total_time = 0;
        float lap_run_total_grade = 0;
        float lap_run_total_time = 0;
        int sprint_num_results = 0;
        int lap_run_num_results = 0;
        while (sqlite3_step(result_stmt) == SQLITE_ROW) {
            if (sqlite3_column_int(result_stmt, 4) == RUN_TYPE_SPRINT) {
                sprint_total_grade += sqlite3_column_double(result_stmt, 6);
                sprint_total_time += sqlite3_column_int(result_stmt, 3);
                sprint_num_results++;
            } else {
                lap_run_total_grade += sqlite3_column_double(result_stmt, 6);
                lap_run_total_time += sqlite3_column_int(result_stmt, 3);
                lap_run_num_results++;
            }
        }
        if (sprint_num_results == 0) {
            student.sprint_avg_grade = 0;
            student.sprint_avg_time = 0;
        } else {
            student.sprint_avg_grade = sprint_total_grade / sprint_num_results;
            student.sprint_avg_time = sprint_total_time / sprint_num_results;
        }
        if (lap_run_num_results == 0) {
            student.lap_run_avg_grade = 0;
            student.lap_run_avg_time = 0;
        } else {
            student.lap_run_avg_grade = lap_run_total_grade / lap_run_num_results;
            student.lap_run_avg_time = lap_run_total_time / lap_run_num_results;
        }
        sqlite3_finalize(result_stmt);

        String run_sql = "SELECT * FROM runs WHERE class_id = " + String(sqlite3_column_int(student_stmt, 3)) + ";";
        sqlite3_stmt* run_stmt;
        if (sqlite3_prepare_v2(this->db, run_sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(run_stmt);
            break;
        }
        while (sqlite3_step(run_stmt) == SQLITE_ROW) {
            StudentInfoRun run;

            run.run_id = sqlite3_column_int(run_stmt, 0);
            run.type = sqlite3_column_int(run_stmt, 1);
            run.length = sqlite3_column_int(run_stmt, 7);
            run.date = sqlite3_column_int(run_stmt, 2);
            
            sqlite3_stmt* result_stmt;
            String result_sql = "SELECT * FROM results WHERE run_id = " + String(sqlite3_column_int(run_stmt, 0)) + " AND student_id = " + String(student_id) + ";";
            if (sqlite3_prepare_v2(this->db, result_sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
                DEBUG_SER_PRINT("Failed to prepare statement: ");
                DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                sqlite3_finalize(result_stmt);
                break;
            }
            if (sqlite3_step(result_stmt) == SQLITE_ROW) {
                run.grade = sqlite3_column_double(result_stmt, 6);
                run.time = sqlite3_column_int(result_stmt, 3);
            } else {
                run.grade = 0;
                run.time = 0;
            }
            sqlite3_finalize(result_stmt);
            student.runs.push_back(run);
        }
    }

    sqlite3_finalize(student_stmt);
    return student;
};

int Database::deleteStudent(int student_id) {
    String sql = "DELETE FROM students WHERE id = " + String(student_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete student: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    return DB_SUCCESS;
}

int Database::putStudent(String name, int gender, int class_id) {
    String sql = "SELECT id FROM students WHERE name = '" + name + "';";

    sqlite3_stmt* student_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(student_stmt) == SQLITE_ROW) {
        sqlite3_finalize(student_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(student_stmt);

    sql = "SELECT id FROM classes WHERE id = " + String(class_id) + ";";
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(class_stmt) != SQLITE_ROW) {
        sqlite3_finalize(class_stmt);
        return DB_NOT_FOUND;
    }
    sqlite3_finalize(class_stmt);

    sql = "INSERT INTO students (name, gender, class_id) VALUES ('" + name + "', " + String(gender) + ", " + String(class_id) + ");";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to insert student: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        if (sqlite3_errcode(this->db) == SQLITE_CONSTRAINT) {
            return DB_CONFLICT;
        } else {
            return DB_FAILED;
        }
    }
    int student_id = sqlite3_last_insert_rowid(this->db);
    return student_id;
}

int Database::patchStudent(int id, String name, int class_id) {
    String sql = "SELECT * FROM students WHERE id = '" + String(id) + "';";

    sqlite3_stmt* student_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(student_stmt) != SQLITE_ROW) {
        sqlite3_finalize(student_stmt);
        return DB_NOT_FOUND;
    }
    sqlite3_finalize(student_stmt);

    sql = "SELECT * FROM students WHERE name = '" + name + "' AND class_id = " + String(class_id) + ";";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(student_stmt) == SQLITE_ROW) {
        sqlite3_finalize(student_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(student_stmt);

    sql = "UPDATE students SET ";
    if (name != "") {
        sql += "name = '" + name + "'";
    }
    if (class_id != -1) {
        if (name != "") {
            sql += ", ";
        }
        sql += "class_id = " + String(class_id);
    }
    sql += " WHERE id = " + String(id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to update student: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    return DB_SUCCESS;
}

std::vector<GradingKeySimple> Database::getGradingKeys() {
    std::vector<GradingKeySimple> grading_keys;

    String sql = "SELECT * FROM grading_keys;";

    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return grading_keys;
    }
    while(sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        GradingKeySimple grading_key;
        grading_key.name = String((const char*) sqlite3_column_text(grading_key_stmt, 1));
        grading_key.type = sqlite3_column_int(grading_key_stmt, 2);
        grading_key.length = sqlite3_column_int(grading_key_stmt, 3);
        grading_key.gender = sqlite3_column_int(grading_key_stmt, 4);
        grading_key.id = sqlite3_column_int(grading_key_stmt, 0);

        String grading_key_grades_sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key.id) + ";";
        sqlite3_stmt* grading_key_grades_stmt;
        if (sqlite3_prepare_v2(this->db, grading_key_grades_sql.c_str(), -1, &grading_key_grades_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(grading_key_grades_stmt);
            break;
        }
        int min_time = INT32_MAX;
        while(sqlite3_step(grading_key_grades_stmt) == SQLITE_ROW) {
            if (sqlite3_column_int(grading_key_grades_stmt, 1) < min_time) {
                min_time = sqlite3_column_int(grading_key_grades_stmt, 1);
            }
        }
        sqlite3_finalize(grading_key_grades_stmt);
        grading_key.min_time = min_time;

        grading_keys.push_back(grading_key);
    }

    sqlite3_finalize(grading_key_stmt);
    return grading_keys;
}

GradingKey Database::getGradingKey(int grading_key_id) {
    GradingKey grading_key;
    grading_key.type = -1;

    String sql = "SELECT * FROM grading_keys WHERE id = " + String(grading_key_id) + ";";
    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return grading_key;
    }
    if (sqlite3_step(grading_key_stmt) != SQLITE_ROW) {
        sqlite3_finalize(grading_key_stmt);
        return grading_key;
    }

    grading_key.name = String((const char*) sqlite3_column_text(grading_key_stmt, 1));
    grading_key.type = sqlite3_column_int(grading_key_stmt, 2);
    grading_key.length = sqlite3_column_int(grading_key_stmt, 3);
    grading_key.gender = sqlite3_column_int(grading_key_stmt, 4);
    sqlite3_finalize(grading_key_stmt);

    sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key_id) + ";";
    sqlite3_stmt* grading_key_grades_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_grades_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_grades_stmt);
        return grading_key;
    }

    while (sqlite3_step(grading_key_grades_stmt) == SQLITE_ROW) {
        GradingKeyGrade grade;
        grade.time = sqlite3_column_int(grading_key_grades_stmt, 1);
        grade.grade = sqlite3_column_double(grading_key_grades_stmt, 2);
        grading_key.grades.push_back(grade);
    }

    sqlite3_finalize(grading_key_grades_stmt);
    return grading_key;
}

GradingKeyMap Database::getGradingKeyMap(int type, int length) {
    GradingKeyMap grading_key_map;

    String sql = "SELECT * FROM grading_keys WHERE type = " + String(type) + " AND length = " + String(length) + ";";
    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return grading_key_map;
    }

    while (sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        GradingKeyIdMap grading_key_id_map;
        grading_key_id_map.name = (const char*) sqlite3_column_text(grading_key_stmt, 1);
        grading_key_id_map.id = sqlite3_column_int(grading_key_stmt, 0);
        if (sqlite3_column_int(grading_key_stmt, 4) == GENDER_TYPE_MALE) {
            grading_key_map.males.push_back(grading_key_id_map);
        } else {
            grading_key_map.females.push_back(grading_key_id_map);
        }
    }
    sqlite3_finalize(grading_key_stmt);

    return grading_key_map;
}

int Database::deleteGradingKey(int grading_key_id) {
    String sql = "SELECT id FROM grading_keys where id = " + String(grading_key_id) + ";";
    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(grading_key_stmt) != SQLITE_ROW) {
        sqlite3_finalize(grading_key_stmt);
        return DB_NOT_FOUND;
    }
    sqlite3_finalize(grading_key_stmt);

    sql = "DELETE FROM grading_keys WHERE id = " + String(grading_key_id) + ";";
    
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete grading key: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    return DB_SUCCESS;
}

int Database::putGradingKey(String name, int type, int length, int gender, std::vector<GradingKeyGrade> grades) {
    String sql = "SELECT id FROM grading_keys WHERE name = '" + name + "';";
    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        sqlite3_finalize(grading_key_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(grading_key_stmt);
    
    sql = "INSERT INTO grading_keys (name, type, length, gender) VALUES  ('" + name + "', " + String(type) + ", " + String(length) + ", " + String(gender) + ");";

    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to insert grading key: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        if (sqlite3_errcode(this->db) == SQLITE_CONSTRAINT) {
            return DB_CONFLICT;
        } else {
            return DB_FAILED;
        }
    }
    int grading_key_id = sqlite3_last_insert_rowid(this->db);
    for (int i = 0; i < grades.size(); i++) {
        sql = "INSERT INTO grading_keys_grades (grading_key_id, time, grade) VALUES (" + String(grading_key_id) + ", " + String(grades[i].time) + ", " + String(grades[i].grade) + ");";
        if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to insert grading key grade: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }
    return grading_key_id;
}

int Database::patchGradingKey(int id, String name, int length, std::vector<GradingKeyGrade> grades) {
    String sql = "SELECT * FROM grading_keys WHERE id = " + String(id) + ";";
    sqlite3_stmt* grading_key_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(grading_key_stmt) != SQLITE_ROW) {
        sqlite3_finalize(grading_key_stmt);
        return DB_NOT_FOUND;
    }
    sqlite3_finalize(grading_key_stmt);

    sql = "SELECT * FROM grading_keys WHERE name = '" + name + "';";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        sqlite3_finalize(grading_key_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(grading_key_stmt);

    sql = "UPDATE grading_keys SET ";
    if (name != "") {
        sql += "name = '" + name + "'";
    }
    if (length != -1) {
        if (name != "") {
            sql += ", ";
        }
        sql += "length = " + String(length);
    }
    sql += " WHERE id = " + String(id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to update grading key: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sql = "SELECT * FROM grading_keys_grades WHERE grading_key_id = " + String(id) + ";";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &grading_key_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(grading_key_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(grading_key_stmt) == SQLITE_ROW) {
        float grade = sqlite3_column_double(grading_key_stmt, 2);
        float time = sqlite3_column_int(grading_key_stmt, 1);
        int id = sqlite3_column_int(grading_key_stmt, 0);

        for (int i = 0; i < grades.size(); i++) {
            if (grades[i].grade == grade) {
                if (grades[i].time != time) {
                    sql = "UPDATE grading_keys_grades SET time = " + String(grades[i].time) + " WHERE grading_key_id = " + String(id) + " AND grade = " + String(grade) + ";";
                    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
                        DEBUG_SER_PRINT("Failed to update grading key grade: ");
                        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                        return DB_FAILED;
                    }
                }
            }
        }
    }
    sqlite3_finalize(grading_key_stmt);
    return DB_SUCCESS;
}