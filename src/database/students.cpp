#include <database.h>

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
        double sprint_total_grade = 0;
        double sprint_total_time = 0;
        double lap_run_total_grade = 0;
        double lap_run_total_time = 0;
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
        if (student.sprint_avg_grade == 0) {
            student.global_avg_grade = student.lap_run_avg_grade;
        } else if (student.lap_run_avg_grade == 0) {
            student.global_avg_grade = student.sprint_avg_grade;
        } else {
            student.global_avg_grade = (student.sprint_avg_grade + student.lap_run_avg_grade) / 2;
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
            
            sqlite3_stmt* result_stmt;
            String result_sql = "SELECT * FROM results WHERE run_id = " + String(sqlite3_column_int(run_stmt, 0)) + " AND student_id = " + String(student_id) + ";";
            if (sqlite3_prepare_v2(this->db, result_sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
                DEBUG_SER_PRINT("Failed to prepare statement: ");
                DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                sqlite3_finalize(result_stmt);
                break;
            }
            if (sqlite3_step(result_stmt) == SQLITE_ROW) {
                run.run_id = sqlite3_column_int(run_stmt, 0);
                run.type = sqlite3_column_int(run_stmt, 1);
                run.length = sqlite3_column_int(run_stmt, 7);
                run.date = sqlite3_column_int(run_stmt, 2);
                run.grade = sqlite3_column_double(result_stmt, 6);
                run.time = sqlite3_column_int(result_stmt, 3);
                student.runs.push_back(run);
            }
            sqlite3_finalize(result_stmt);
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
 * Deletes all students and their associated runs from the system
 * @param password The admin password as validation of this action
 * @return A status code defined in database.h
*/
int Database::deleteStudents(String password) {
    String sql = "SELECT * FROM teachers WHERE administrator = 1;";
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
    String stored_password = String((const char*) sqlite3_column_text(teacher_stmt, 3));
    sqlite3_finalize(teacher_stmt);

    if (password != stored_password) {
        return DB_FAILED;
    }
    
    sql = "DELETE FROM runs;";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete runs: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sql = "DELETE FROM students;";
    if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to delete students: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        return DB_FAILED;
    }

    sql = "SELECT * FROM classes;";
    sqlite3_stmt* class_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &class_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(class_stmt);
        return DB_FAILED;
    }
    while (sqlite3_step(class_stmt) == SQLITE_ROW) {
        int class_id = sqlite3_column_int(class_stmt, 0);
        sql = "UPDATE classes SET sprint_avg_grade = 6, sprint_avg_time = 0, lap_run_avg_grade = 6, lap_run_avg_time = 0 WHERE id = " + String(class_id) + ";";
        if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to update class: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            return DB_FAILED;
        }
    }
    sqlite3_finalize(class_stmt);

    return DB_SUCCESS;
}