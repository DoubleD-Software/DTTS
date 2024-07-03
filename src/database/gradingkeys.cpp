#include <database.h>

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
        double grade = sqlite3_column_double(grading_key_stmt, 2);
        double time = sqlite3_column_int(grading_key_stmt, 1);
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