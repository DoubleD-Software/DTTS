#include <database.h>

/**
 * Table layout for the database
 * Gets converted to SQL and executed in the createTables function
*/
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
        SqlColumn("teacher_id", SQL_TYPE_INT),
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

/**
 * Constructor for the Database class
 * @param db_path The path to the database file
*/
Database::Database(String db_path) {
    this->db_path = db_path;
}

/**
 * Opens the database and creates the tables if they don't exist
*/
void Database::open() {
    if (sqlite3_open(this->db_path.c_str(), &this->db) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to open database: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sysHalt(DB_OPEN_FAILED, "Failed to open database.");
    }

    createTables();
}

/**
 * Closes the database
*/
void Database::close() {
    sqlite3_close(this->db);
}

/**
 * Creates the tables in the database using the TABLES constant
*/
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

    String sql = "SELECT id FROM teachers WHERE name = '" + String(ADMIN_USERNAME) + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(stmt);
        sysHalt(DB_CREATE_FAILED, "Failed to prepare statement.");
    }
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        DEBUG_SER_PRINTLN("Creating admin teacher.");
        sql = "INSERT INTO teachers (name, username, password, administrator) VALUES ('" + String(ADMIN_USERNAME) + "', '" + String(ADMIN_USERNAME) + "', '" + String(ADMIN_PASSWORD) + "', 1);";
        if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to insert admin teacher: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sysHalt(DB_CREATE_FAILED, "Failed to create admin teacher.");
        }
    }
    sqlite3_finalize(stmt);
}

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
            student.gender = sqlite3_column_int(result_stmt, 4);
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
    sqlite3_finalize(run_stmt);

    sql = "DELETE FROM runs WHERE id = " + String(run_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete run: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    // Due to a bug in the sqlite3 library, we can't rely on the ON DELETE CASCADE foreign key constraint
    sql = "SELECT id FROM results WHERE run_id = " + String(run_id) + ";";
    sqlite3_stmt* result_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(result_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(result_stmt) == SQLITE_ROW) {
        int result_id = sqlite3_column_int(result_stmt, 0);
        String lap_sql = "DELETE FROM laps WHERE result_id = " + String(result_id) + ";";
        if (sqlite3_exec(this->db, lap_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to delete laps: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }
    sqlite3_finalize(result_stmt);
    
    sql = "DELETE FROM results WHERE run_id = " + String(run_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete results: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sql = "DELETE FROM participants WHERE run_id = " + String(run_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete participants: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    // The above code may be removed if the ON DELETE CASCADE foreign key constraint is fixed

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

/**
 * Fetches a student's information from the database
 * @param student_id The ID of the student
 * @return A StudentInfo object
*/
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

/**
 * Deletes a student from the database
 * @param student_id The ID of the student to delete
 * @return A status code defined in database.h
*/
int Database::deleteStudent(int student_id) {
    String sql = "DELETE FROM students WHERE id = " + String(student_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete student: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    // Due to a bug in the sqlite3 library, we can't rely on the ON DELETE CASCADE foreign key constraint
    sql = "SELECT id FROM results WHERE student_id = " + String(student_id) + ";";
    sqlite3_stmt* result_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(result_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(result_stmt) == SQLITE_ROW) {
        int result_id = sqlite3_column_int(result_stmt, 0);
        String lap_sql = "DELETE FROM laps WHERE result_id = " + String(result_id) + ";";
        if (sqlite3_exec(this->db, lap_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to delete laps: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }
    sqlite3_finalize(result_stmt);

    sql = "DELETE FROM results WHERE student_id = " + String(student_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete results: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sql = "DELETE FROM participants WHERE student_id = " + String(student_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete participants: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    // The above code may be removed if the ON DELETE CASCADE foreign key constraint is fixed

    return DB_SUCCESS;
}

/**
 * Creates a new student in the database
 * @param name The name of the student
 * @param gender The gender of the student
 * @param class_id The ID of the class the student is in
 * @return The ID of the new student, or a status code defined in database.h
*/
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

/**
 * Edits a student in the database
 * @param id The ID of the student to edit
 * @param name The new name of the student or "" to keep the same
 * @param class_id The new class ID of the student or -1 to keep the same
*/
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

/**
 * Fetches all grading keys from the database
 * @return A vector of GradingKeySimple objects
*/
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

/**
 * Fetches the information for a specific grading key
 * @param grading_key_id The ID of the grading key
 * @return A GradingKey object; if the grading key is not found, the type will be -1
*/
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

/**
 * Fetches the grading key map for a specific type and length
 * @param type The type of the grading key; type in this context refers to the gender
 * @param length The length of the grading key
*/
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

/**
 * Deletes a grading key from the database
 * @param grading_key_id The ID of the grading key to delete
 * @return A status code defined in database.h
*/
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

    // Due to a bug in the sqlite3 library, we can't rely on the ON DELETE CASCADE foreign key constraint
    sql = "DELETE FROM grading_keys_grades WHERE grading_key_id = " + String(grading_key_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete grading key grades: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    // The above code may be removed if the ON DELETE CASCADE foreign key constraint is fixed

    return DB_SUCCESS;
}

/**
 * Creates a new grading key in the database
 * @param name The name of the grading key
 * @param type The type of the grading key
 * @param length The length of the results the grading key is for
 * @param gender The gender the grading key is for
 * @param grades A vector of GradingKeyGrade objects
 * @return The ID of the new grading key, or a status code defined in database.h
*/
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

/**
 * Edits a grading key in the database
 * @param id The ID of the grading key to edit
 * @param name The new name of the grading key or "" to keep the same
 * @param length The new length of the grading key or -1 to keep the same
 * @param grades A vector of GradingKeyGrade objects
*/
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

/**
 * Fetches all classes from the database
 * @return A vector of ClassInfoSimple objects
*/
std::vector<ClassInfoSimple> Database::getClasses() {
    std::vector<ClassInfoSimple> classes;

    String sql = "SELECT * FROM classes;";

    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return classes;
    }
    while(sqlite3_step(class_stmt) == SQLITE_ROW) {
        ClassInfoSimple class_info;
        class_info.name = String((const char*) sqlite3_column_text(class_stmt, 1));
        class_info.class_id = sqlite3_column_int(class_stmt, 0);
        class_info.sprint_avg_grade = sqlite3_column_double(class_stmt, 2);
        class_info.sprint_avg_time = sqlite3_column_int(class_stmt, 3);
        class_info.lap_run_avg_grade = sqlite3_column_double(class_stmt, 4);
        class_info.lap_run_avg_time = sqlite3_column_int(class_stmt, 5);

        sql = "SELECT id FROM students WHERE class_id = " + String(class_info.class_id) + ";";
        sqlite3_stmt* student_stmt;
        if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(student_stmt);
            break;
        }

        while (sqlite3_step(student_stmt) == SQLITE_ROW) {
            class_info.size++;
        }
        classes.push_back(class_info);
        sqlite3_finalize(student_stmt);
    }

    sqlite3_finalize(class_stmt);
    return classes;
}

/**
 * Fetches the information for a specific class
 * @param class_id The ID of the class
 * @return A ClassInfo object
*/
ClassInfo Database::getClassInfo(int class_id) {
    ClassInfo class_info;

    String sql = "SELECT * FROM classes WHERE id = " + String(class_id) + ";";
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return class_info;
    }
    if (sqlite3_step(class_stmt) == SQLITE_ROW) {
        class_info.name = String((const char*) sqlite3_column_text(class_stmt, 1));
        class_info.sprint_avg_grade = sqlite3_column_double(class_stmt, 2);
        class_info.sprint_avg_time = sqlite3_column_int(class_stmt, 3);
        class_info.lap_run_avg_grade = sqlite3_column_double(class_stmt, 4);
        class_info.lap_run_avg_time = sqlite3_column_int(class_stmt, 5);
    } else {
        sqlite3_finalize(class_stmt);
        return class_info;
    }
    sqlite3_finalize(class_stmt);

    sql = "SELECT * FROM students WHERE class_id = " + String(class_id) + ";";
    sqlite3_stmt* student_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return class_info;
    }
    while (sqlite3_step(student_stmt) == SQLITE_ROW) {
        ClassInfoStudent student;
        student.name = String((const char*) sqlite3_column_text(student_stmt, 1));
        student.gender = sqlite3_column_int(student_stmt, 2);
        student.student_id = sqlite3_column_int(student_stmt, 0);
        
        sql = "SELECT * FROM results WHERE student_id = " + String(sqlite3_column_int(student_stmt, 0)) + ";";
        sqlite3_stmt* result_stmt;
        if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(result_stmt);
            break;
        }
        float avg_grade = 0;
        int num_results = 0;
        while (sqlite3_step(result_stmt) == SQLITE_ROW) {
            avg_grade += sqlite3_column_double(result_stmt, 6);
            num_results++;
        }
        if (num_results == 0) {
            student.avg_grade = 0;
        } else {
            student.avg_grade = avg_grade / num_results;
        }
        sqlite3_finalize(result_stmt);
        class_info.students.push_back(student);
    }
    sqlite3_finalize(student_stmt);
    
    sql = "SELECT * FROM runs WHERE class_id = " + String(class_id) + ";";
    sqlite3_stmt* run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        return class_info;
    }
    while (sqlite3_step(run_stmt) == SQLITE_ROW) {
        ClassInfoRun run;
        run.run_id = sqlite3_column_int(run_stmt, 0);
        run.type = sqlite3_column_int(run_stmt, 1);
        run.date = sqlite3_column_int(run_stmt, 2);
        run.length = sqlite3_column_int(run_stmt, 7);

        sql = "SELECT * FROM results WHERE run_id = " + String(run.run_id) + ";";
        sqlite3_stmt* result_stmt;
        if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
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
        class_info.runs.push_back(run);
    }
    sqlite3_finalize(run_stmt);

    return class_info;
}

/**
 * Deletes a class from the database and all students and runs associated with it
 * @param class_id The ID of the class to delete
 * @return A status code defined in database.h
*/
int Database::deleteClass(int class_id) {
    String sql = "SELECT id FROM classes where id = " + String(class_id) + ";";
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

    sql = "DELETE FROM classes WHERE id = " + String(class_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete class: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    // Due to a bug in the sqlite3 library, we can't rely on the ON DELETE CASCADE foreign key constraint
    sql = "SELECT id FROM students WHERE class_id = " + String(class_id) + ";";
    sqlite3_stmt* student_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(student_stmt) == SQLITE_ROW) {
        int student_id = sqlite3_column_int(student_stmt, 0);
        String result_sql = "SELECT id FROM results WHERE student_id = " + String(student_id) + ";";
        sqlite3_stmt* result_stmt;
        if (sqlite3_prepare_v2(this->db, result_sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(result_stmt);
            return DB_FAILED;
        }
        while (sqlite3_step(result_stmt) == SQLITE_ROW) {
            int result_id = sqlite3_column_int(result_stmt, 0);
            String lap_sql = "DELETE FROM laps WHERE result_id = " + String(result_id) + ";";
            if (sqlite3_exec(this->db, lap_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
                DEBUG_SER_PRINT("Failed to delete laps: ");
                DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                return DB_FAILED;
            }
        }
        sqlite3_finalize(result_stmt);

        result_sql = "DELETE FROM results WHERE student_id = " + String(student_id) + ";";
        if (sqlite3_exec(this->db, result_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to delete results: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }

        String participant_sql = "DELETE FROM participants WHERE student_id = " + String(student_id) + ";";
        if (sqlite3_exec(this->db, participant_sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to delete participants: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }

    sql = "DELETE FROM students WHERE class_id = " + String(class_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete students: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sql = "DELETE FROM runs WHERE class_id = " + String(class_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete runs: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    // The above code may be removed if the ON DELETE CASCADE foreign key constraint is fixed

    return DB_SUCCESS;
}

/**
 * Creates a new class in the database
 * @param name The name of the class
 * @return The ID of the new class, or a status code defined in database.h
*/
int Database::putClass(String name) {
    String sql = "SELECT id FROM classes WHERE name = '" + name + "';";
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(class_stmt) == SQLITE_ROW) {
        sqlite3_finalize(class_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(class_stmt);

    sql = "INSERT INTO classes (name) VALUES ('" + name + "');";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to insert class: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        if (sqlite3_errcode(this->db) == SQLITE_CONSTRAINT) {
            return DB_CONFLICT;
        } else {
            return DB_FAILED;
        }
    }
    int class_id = sqlite3_last_insert_rowid(this->db);
    return class_id;
}

/**
 * Edits a class in the database
 * @param class_id The ID of the class to edit
 * @param name The new name of the class
 * @return A status code defined in database.h
*/
int Database::patchClass(int class_id, String name) {
    String sql = "SELECT * FROM classes WHERE id = " + String(class_id) + ";";
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

    sql = "SELECT * FROM classes WHERE name = '" + name + "';";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(class_stmt) == SQLITE_ROW) {
        sqlite3_finalize(class_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(class_stmt);

    sql = "UPDATE classes SET name = '" + name + "' WHERE id = " + String(class_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to update class: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    return DB_SUCCESS;
}

/**
 * Fetches all students in a class
 * @param class_id The ID of the class
 * @return A vector of ClassStudent objects
*/
std::vector<ClassStudent> Database::getClassStudents(int class_id) {
    std::vector<ClassStudent> students;

    String sql = "SELECT * FROM students WHERE class_id = " + String(class_id) + ";";
    sqlite3_stmt* student_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &student_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(student_stmt);
        return students;
    }
    while (sqlite3_step(student_stmt) == SQLITE_ROW) {
        ClassStudent student;
        student.student_id = sqlite3_column_int(student_stmt, 0);
        student.name = String((const char*) sqlite3_column_text(student_stmt, 1));
        students.push_back(student);
    }
    sqlite3_finalize(student_stmt);
    return students;
}

/**
 * Fetches all class names and IDs
 * @return A vector of ClassName objects
*/
std::vector<ClassName> Database::getClassNames() {
    std::vector<ClassName> classes;

    String sql = "Select * FROM classes;";
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return classes;
    }
    while (sqlite3_step(class_stmt) == SQLITE_ROW) {
        ClassName class_name;
        class_name.class_id = sqlite3_column_int(class_stmt, 0);
        class_name.name = String((const char*) sqlite3_column_text(class_stmt, 1));
        classes.push_back(class_name);
    }
    sqlite3_finalize(class_stmt);
    return classes;
}

/**
 * Fetches all teachers from the database
 * @return A vector of TeacherSimple objects
*/
std::vector<TeacherSimple> Database::getTeachers() {
    std::vector<TeacherSimple> teachers;

    String sql = "SELECT * FROM teachers;";
    sqlite3_stmt* teacher_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return teachers;
    }
    while (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
        TeacherSimple teacher;
        teacher.name = String((const char*) sqlite3_column_text(teacher_stmt, 1));
        teacher.teacher_id = sqlite3_column_int(teacher_stmt, 0);
        teachers.push_back(teacher);
    }
    sqlite3_finalize(teacher_stmt);
    return teachers;
}

/**
 * Fetches the information for a specific teacher
 * @param teacher_id The ID of the teacher
 * @return A Teacher object; if the teacher is not found, the username will be ""
*/
Teacher Database::getTeacher(int teacher_id) {
    Teacher teacher;

    String sql = "SELECT * FROM teachers WHERE id = " + String(teacher_id) + ";";
    sqlite3_stmt* teacher_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return teacher;
    }
    if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
        teacher.name = String((const char*) sqlite3_column_text(teacher_stmt, 1));
        teacher.username = String((const char*) sqlite3_column_text(teacher_stmt, 2));
    } else {
        sqlite3_finalize(teacher_stmt);
        return teacher;
    }
    sqlite3_finalize(teacher_stmt);
    return teacher;
}

/**
 * Deletes a teacher from the database
 * @param teacher_id The ID of the teacher to delete
 * @return A status code defined in database.h
 * @note This function doesn't delete associated runs
*/
int Database::deleteTeacher(int teacher_id) {
    String sql = "SELECT id FROM teachers where id = " + String(teacher_id) + ";";
    sqlite3_stmt* teacher_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(teacher_stmt) != SQLITE_ROW) {
        sqlite3_finalize(teacher_stmt);
        return DB_NOT_FOUND;
    }
    sqlite3_finalize(teacher_stmt);

    sql = "DELETE FROM teachers WHERE id = " + String(teacher_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete teacher: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    return DB_SUCCESS;
}

/**
 * Creates a new teacher in the database
 * @param name The name of the teacher
 * @param username The username of the teacher
 * @param password The password of the teacher
 * @return The ID of the new teacher, or a status code defined in database.h
*/
int Database::putTeacher(String name, String username, String password) {
    String sql = "SELECT id FROM teachers WHERE username = '" + username + "';";
    sqlite3_stmt* teacher_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
        sqlite3_finalize(teacher_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(teacher_stmt);

    sql = "INSERT INTO teachers (name, username, password, administrator) VALUES ('" + name + "', '" + username + "', '" + password + "', 0);";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to insert teacher: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        if (sqlite3_errcode(this->db) == SQLITE_CONSTRAINT) {
            return DB_CONFLICT;
        } else {
            return DB_FAILED;
        }
    }
    int teacher_id = sqlite3_last_insert_rowid(this->db);
    return teacher_id;
}

/**
 * Edits a teacher in the database
 * @param teacher_id The ID of the teacher to edit
 * @param name The new name of the teacher or "" to keep the same
 * @param username The new username of the teacher or "" to keep the same
 * @param password The new password of the teacher or "" to keep the same
 * @return A status code defined in database.h
*/
int Database::patchTeacher(int teacher_id, String name, String username, String password) {
    String sql = "SELECT * FROM teachers WHERE id = " + String(teacher_id) + ";";
    sqlite3_stmt* teacher_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(teacher_stmt) != SQLITE_ROW) {
        sqlite3_finalize(teacher_stmt);
        return DB_NOT_FOUND;
    }
    sqlite3_finalize(teacher_stmt);

    sql = "SELECT * FROM teachers WHERE username = '" + username + "';";
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &teacher_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(teacher_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(teacher_stmt) == SQLITE_ROW) {
        sqlite3_finalize(teacher_stmt);
        return DB_CONFLICT;
    }
    sqlite3_finalize(teacher_stmt);

    sql = "UPDATE teachers SET ";
    if (name != "") {
        sql += "name = '" + name + "'";
    }
    if (username != "") {
        if (name != "") {
            sql += ", ";
        }
        sql += "username = '" + username + "'";
    }
    if (password != "") {
        if (name != "" || username != "") {
            sql += ", ";
        }
        sql += "password = '" + password + "'";
    }
    sql += " WHERE id = " + String(teacher_id) + ";";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to update teacher: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }
    return DB_SUCCESS;
}

/**
 * Checks if a username and password are valid
 * @param username The username to check
 * @param password The password to check
 * @return A status code defined in database.h
*/
int Database::checkPassword(String username, String password) {
    String sql = "SELECT * FROM teachers WHERE username = '" + username + "';";
    sqlite3_stmt* user_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &user_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(user_stmt);
        return DB_FAILED;
    }
    if (sqlite3_step(user_stmt) != SQLITE_ROW) {
        sqlite3_finalize(user_stmt);
        return DB_NOT_FOUND;
    }
    String stored_password = String((const char*) sqlite3_column_text(user_stmt, 3));
    int admin = sqlite3_column_int(user_stmt, 4);
    sqlite3_finalize(user_stmt);

    if (password == stored_password) {
        return admin;
    } else {
        return DB_INVALID;
    }
}