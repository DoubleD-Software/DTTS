#include <sql.h>

SQL::SQL() {
}

/**
 * Initialize SQLite3 and open the database. This function has to be called before any other functions of this class.
 * In case of an error, the system will halt, with an error message on the serial.
 * @param db_path The path to the database file.
*/
void SQL::begin(const char *db_path) {
    DEBUG_SER_PRINTLN("Initializing SQLite3...");
    sqlite3_initialize();
    openDB(db_path);
    DEBUG_SER_PRINTLN("SQLite3 initialized.");
}

/**
 * Open a database without initializing SQLite3.
 * In case of an error, the system will halt, with an error message on the serial.
 * @param db_path The path to the database file. If the file is on the sd card, the path has to start with "/sd".
*/
void SQL::openDB(const char *db_path) {
    int rc = sqlite3_open(db_path, &db);
    if (rc) {
        DEBUG_SER_PRINTLN("Failed to open database");
        sysHalt(1, "Database open failed.");
    } else {
        DEBUG_SER_PRINTLN("Opened database successfully");
    }
}

/**
 * Close the database.
*/
void SQL::closeDB() {
    sqlite3_close(db);
}

/**
 * Clear the given table in the Database. This function is currently vunerable to SQL injection. Has to be fixed in the future.
 * @param table_name The name of the table to clear. Has to be maximum of 128 characters long.
 * @return 0 on success, error code on failure.
*/
int SQL::clearTable(const char *table_name) {
    int table_name_size = strlen(table_name);
    if (table_name_size > 128) {
        DEBUG_SER_PRINTLN("Invalid Table Name. Name too long.");
        return 1;
    }
    char sql[12 + table_name_size + 1] = {0}; // 12 characters for "DELETE FROM ", table_name_size characters for the table name and 1 character for the null terminator.
    sprintf(sql, "DELETE FROM %s", table_name);
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc) {
        DEBUG_SER_PRINTLN("Failed to clear table.");
        DEBUG_SER_PRINTLN(err_msg);
        sqlite3_free(err_msg);
        return rc;
    } else {
        DEBUG_SER_PRINTLN("Table cleared successfully.");
        return rc;
    }
}

/**
 * Drop the given table in the Database. This function is currently vunerable to SQL injection. Has to be fixed in the future.
 * @param table_name The name of the table to drop. Has to be maximum of 128 characters long.
 * @return 0 on success, error code on failure.
*/
int SQL::dropTable(const char *table_name) {
    int table_name_size = strlen(table_name);
    if (table_name_size > 128) {
        DEBUG_SER_PRINTLN("Invalid Table Name. Name too long.");
        return 1;
    }
    char sql[10 + table_name_size + 1] = {0}; // 10 characters for "DROP TABLE ", table_name_size characters for the table name and 1 character for the null terminator.
    sprintf(sql, "DROP TABLE %s", table_name);
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc) {
        DEBUG_SER_PRINTLN("Failed to drop table.");
        DEBUG_SER_PRINTLN(err_msg);
        sqlite3_free(err_msg);
        return rc;
    } else {
        DEBUG_SER_PRINTLN("Table dropped successfully.");
        return rc;
    }
}

/**
 * Execute a SQL statement.
 * @param sql The SQL statement to execute.
 * @return 0 on success, error code on failure.
*/
int SQL::dbExec(const char *sql) {
    DEBUG_SER_PRINT("Executing statement: ");
    DEBUG_SER_PRINTLN(sql);
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc) {
        DEBUG_SER_PRINTLN("Failed to insert into table.");
        DEBUG_SER_PRINTLN(err_msg);
        sqlite3_free(err_msg);
        return rc;
    } else {
        DEBUG_SER_PRINTLN("Inserted into table successfully.");
        return rc;
    }
}

/**
 * Create a table in the database. This function is currently vunerable to SQL injection. Has to be fixed in the future.
 * @param table_name The name of the table to create. Has to be maximum of 128 characters long.
 * @param columns An array of sql_column_descriptor_t structs, containing the column names, types and sizes.
 * @param column_count The number of columns in the table.
 * @return 0 on success, error code on failure.
*/
int SQL::createTable(const char *table_name, sql_column_descriptor_t *columns, int column_count) {
    int table_name_size = strlen(table_name);
    if (table_name_size > 128) {
        DEBUG_SER_PRINTLN("Invalid Table Name. Name too long.");
        return 1;
    }
    char sql[13 + table_name_size + 1] = {0}; // 13 characters for "CREATE TABLE ", table_name_size characters for the table name and 1 character for the null terminator.
    sprintf(sql, "CREATE TABLE %s (", table_name);
    for (int i = 0; i < column_count; i++) {
        if (i != 0) {
            strcat(sql, ", ");
        }
        strcat(sql, columns[i].name);
        switch (columns[i].type) {
            case SQL_TYPE_INT: {
                strcat(sql, " INT");
                break;
            }
            case SQL_TYPE_VARCHAR: {
                strcat(sql, " VARCHAR(");
                char size[6] = {0};
                sprintf(size, "%d", columns[i].size);
                strcat(sql, size);
                strcat(sql, ")");
                break;
            }
            default: {
                DEBUG_SER_PRINTLN("Invalid column type.");
                return 1;
            }
        }
        if (columns[i].additional != NULL) {
            strcat(sql, " ");
            strcat(sql, columns[i].additional);
        }
    }
    strcat(sql, ")");
    return dbExec(sql);
}

/**
 * Insert a row into the given table in the database. This function is currently vunerable to SQL injection. Has to be fixed in the future.
 * @param table_name The name of the table to insert into. Has to be maximum of 128 characters long.
 * @param columns An array of sql_column_t structs, containing the column names, types and values.
 * @param column_count The number of columns in the table.
 * @return 0 on success, error code on failure.
*/
int SQL::insertIntoTable(const char *table_name, sql_column_t *columns, int column_count) {
    int table_name_size = strlen(table_name);
    if (table_name_size > 128) {
        DEBUG_SER_PRINTLN("Invalid Table Name. Name too long.");
        return 1;
    }
    char sql[13 + table_name_size + 1] = {0}; // 13 characters for "INSERT INTO ", table_name_size characters for the table name and 1 character for the null terminator.
    sprintf(sql, "INSERT INTO %s (", table_name);
    for (int i = 0; i < column_count; i++) {
        if (i != 0) {
            strcat(sql, ", ");
        }
        strcat(sql, columns[i].name);
    }
    strcat(sql, ") VALUES (");
    for (int i = 0; i < column_count; i++) {
        if (i != 0) {
            strcat(sql, ", ");
        }
        switch (columns[i].type) {
            case SQL_TYPE_INT: {
                char value_int[11] = {0};
                sprintf(value_int, "%d", columns[i].value_int);
                strcat(sql, value_int);
                break;
            }
            case SQL_TYPE_VARCHAR: {
                strcat(sql, "'");
                strcat(sql, columns[i].value_varchar);
                strcat(sql, "'");
                break;
            }
            default: {
                DEBUG_SER_PRINTLN("Invalid column type.");
                return 1;
            }
        }
    }
    strcat(sql, ")");
    return dbExec(sql);
}