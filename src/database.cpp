#include <database.h>

const SqlTable Database::TABLES[] = {
    SqlTable("teachers", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_VC_255),
        SqlColumn("username", SQL_TYPE_VC_255),
        SqlColumn("password", SQL_TYPE_VC_255),
        SqlColumn("administrator", SQL_TYPE_INT)
    }),
    SqlTable("classes", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_VC_255)
    }),
    SqlTable("grading_keys", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_VC_255),
        SqlColumn("type", SQL_TYPE_INT)
    }),
    SqlTable("grading_keys_grades", {
        SqlColumn("grading_key_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "grading_keys", "id"),
        SqlColumn("time", SQL_TYPE_FLOAT),
        SqlColumn("grade", SQL_TYPE_FLOAT)
    }),
    SqlTable("students", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("name", SQL_TYPE_VC_255),
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
        SqlColumn("length", SQL_TYPE_INT)
    }),
    SqlTable("results", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("run_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "runs", "id"),
        SqlColumn("student_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "students", "id"),
        SqlColumn("time", SQL_TYPE_FLOAT),
        SqlColumn("grade", SQL_TYPE_FLOAT)
    }),
    SqlTable("laps", {
        SqlColumn("id", SQL_TYPE_INT, SQL_TYPE_PRIMARY_KEY | SQL_TYPE_AUTO_INCREMENT),
        SqlColumn("lap_num", SQL_TYPE_INT),
        SqlColumn("result_id", SQL_TYPE_INT, SQL_TYPE_FOREIGN_KEY, "results", "id"),
        SqlColumn("time", SQL_TYPE_FLOAT)
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
    for (int i = 0; i < sizeof(TABLES) / sizeof(SqlTable); i++) {
        SqlTable table = TABLES[i];
        String sql = "CREATE TABLE IF NOT EXISTS " + table.name + " (";
        for (int j = 0; j < table.columns.size(); j++) {
            SqlColumn column = table.columns[j];
            sql += column.name;
            switch (column.type) {
                case SQL_TYPE_INT:
                    sql += " INTEGER";
                    break;
                case SQL_TYPE_FLOAT:
                    sql += " FLOAT";
                    break;
                case SQL_TYPE_VC_255:
                    sql += " VARCHAR(255)";
                    break;
            }
            if (column.flags & SQL_TYPE_PRIMARY_KEY) {
                sql += " PRIMARY KEY";
            }
            if (column.flags & SQL_TYPE_AUTO_INCREMENT) {
                sql += " AUTOINCREMENT";
            }
            if (column.flags & SQL_TYPE_FOREIGN_KEY) {
                sql += ", FOREIGN KEY(" + String(column.name) + ") REFERENCES " + column.foreign_table + "(" + column.foreign_column + ") ON DELETE CASCADE ON UPDATE CASCADE";
            }
            if (j < table.columns.size() - 1) {
                sql += ", ";
            }
        }
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
            run.teacher = String((const char*) sqlite3_column_text(teacher_stmt, 1));
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
            run.class_name = String((const char*) sqlite3_column_text(class_stmt, 1));
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
            total_grade += sqlite3_column_double(result_stmt, 4);
            total_time += sqlite3_column_double(result_stmt, 3);
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
        run.length = sqlite3_column_int(run_stmt, 7);
        run.date = sqlite3_column_int(run_stmt, 2);
        sqlite3_stmt* teacher_stmt;
        String teacher_sql = "SELECT name FROM teachers WHERE id = " + String(sqlite3_column_int(run_stmt, 6)) + ";";
        if (sqlite3_prepare_v2(this->db, teacher_sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(teacher_stmt);
            break;
        }
        if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
            run.teacher = String((const char*) sqlite3_column_text(teacher_stmt, 1));
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
            run.class_name = String((const char*) sqlite3_column_text(class_stmt, 1));
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
            run.grading_key_m = String((const char*) sqlite3_column_text(grading_key_m_stmt, 1));
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
            run.grading_key_f = String((const char*) sqlite3_column_text(grading_key_f_stmt, 1));
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
                student.name = String((const char*) sqlite3_column_text(student_stmt, 1));
            } else {
                student.name = UNKNOWN_NAME;
            }
            sqlite3_finalize(student_stmt);
            student.time = sqlite3_column_double(result_stmt, 3);
            student.grade = sqlite3_column_double(result_stmt, 4);
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
        count++;
        student.grade = sqlite3_column_double(result_stmt, 4);
        student.time = sqlite3_column_double(result_stmt, 3);
        student.length = sqlite3_column_int(result_stmt, 5);
        sqlite3_stmt* student_stmt;
        String student_sql = "SELECT name FROM students WHERE id = " + String(student_id) + ";";
        if (sqlite3_prepare_v2(this->db, student_sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(student_stmt);
            break;
        }
        if (sqlite3_step(student_stmt) == SQLITE_ROW) {
            student.student_name = String((const char*) sqlite3_column_text(student_stmt, 1));
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
            lap.lap_number = sqlite3_column_int(lap_stmt, 1);
            lap.length = sqlite3_column_double(lap_stmt, 2);
            lap.time = sqlite3_column_double(lap_stmt, 3);
            student.laps.push_back(lap);
        }
        sqlite3_finalize(lap_stmt);
    }
    return student;
}

int Database::deleteRun(int run_id) {
    String sql = "DELETE FROM runs WHERE id = " + String(run_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete run: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    return DB_SUCCESS;
}

int Database::putRun(int type, int date, int class_id, int grading_key_m_id, int grading_key_f_id, int teacher_id, int length) {
    // Incomplete
    return DB_FAILED;
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
        student.gender = sqlite3_column_int(student_stmt, 2);
        sqlite3_stmt* class_stmt;
        String class_sql = "SELECT name FROM classes WHERE id = " + String(sqlite3_column_int(student_stmt, 3)) + ";";
        if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(class_stmt);
            break;
        }
        if (sqlite3_step(class_stmt) == SQLITE_ROW) {
            student.class_name = String((const char*) sqlite3_column_text(class_stmt, 1));
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
            if (sqlite3_column_int(result_stmt, 1) == RUN_TYPE_SPRINT) {
                sprint_total_grade += sqlite3_column_double(result_stmt, 4);
                sprint_total_time += sqlite3_column_double(result_stmt, 3);
                sprint_num_results++;
            } else {
                lap_run_total_grade += sqlite3_column_double(result_stmt, 4);
                lap_run_total_time += sqlite3_column_double(result_stmt, 3);
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
                run.grade = sqlite3_column_double(result_stmt, 4);
                run.time = sqlite3_column_double(result_stmt, 3);
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

int Database::putStudent(String name, int gender, String class_name) {
    sqlite3_stmt* class_stmt;
    String class_sql = "SELECT id FROM classes WHERE name = '" + class_name + "';";
    if (sqlite3_prepare_v2(this->db, class_sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    int class_id;
    if (sqlite3_step(class_stmt) == SQLITE_ROW) {
        class_id = sqlite3_column_int(class_stmt, 0);
        sqlite3_finalize(class_stmt);
    } else {
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    String student_sql = "INSERT INTO students (name, gender, class_id) VALUES ('" + name + "', " + String(gender) + ", " + String(class_id) + ");";
    if (sqlite3_exec(this->db, student_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
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