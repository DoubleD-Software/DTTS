#include <database.h>

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
        if (class_info.sprint_avg_grade == 0) {
            class_info.global_avg_grade = class_info.lap_run_avg_grade;
        } else if (class_info.lap_run_avg_grade == 0) {
            class_info.global_avg_grade = class_info.sprint_avg_grade;
        } else {
            class_info.global_avg_grade = (class_info.sprint_avg_grade + class_info.lap_run_avg_grade) / 2;
        }
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
        double avg_grade = 0;
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