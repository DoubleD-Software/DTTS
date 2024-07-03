#include <database.h>

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

    sql = "SELECT * FROM teachers WHERE username = '" + username + "' AND id != " + String(teacher_id) + ";";
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