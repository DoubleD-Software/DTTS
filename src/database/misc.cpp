#include <database.h>

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
 * Checks if a username and password are valid
 * @param username The username to check
 * @param password The password to check
 * @return A struct containing the access level and teacher id
*/
UserAuth Database::checkPassword(String username, String password) {
    UserAuth auth;

    String sql = "SELECT * FROM teachers WHERE username = '" + username + "';";
    sqlite3_stmt* user_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &user_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(user_stmt);
        return auth;
    }
    if (sqlite3_step(user_stmt) != SQLITE_ROW) {
        sqlite3_finalize(user_stmt);
        return auth;
    }

    String stored_password = String((const char*) sqlite3_column_text(user_stmt, 3));
    if (stored_password != password) {
        sqlite3_finalize(user_stmt);
        return auth;
    }

    auth.access_level = sqlite3_column_int(user_stmt, 4);
    auth.user_id = sqlite3_column_int(user_stmt, 0);
    sqlite3_finalize(user_stmt);

    return auth;
}

/**
 * Performs a factory reset of the system, deleting the database file
 * @param password The admin password as validation of this action
 * @return A status code defined in database.h
 * @note The system should not handle any other database operations after this and restart asap
*/
int Database::factoryReset(String password) {
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

    close();
    if (!SD.remove("/dtts.db")) {
        DEBUG_SER_PRINTLN("Failed to delete database file");
        sysHalt(SD_FAILED, "Failed to delete db file");
    }

    return DB_SUCCESS;
}