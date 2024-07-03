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

    String sql = "SELECT id FROM teachers WHERE administrator = 1;";
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

    // Cleanup any invalid runs, like runs with no results
    sql = "SELECT * FROM runs;";
    sqlite3_stmt *run_stmt;
    if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &run_stmt, 0) != SQLITE_OK) {
        DEBUG_SER_PRINT("Failed to prepare statement: ");
        DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
        sqlite3_finalize(run_stmt);
        sysWarn("Failed to cleanup db.");
    }

    while (sqlite3_step(run_stmt) == SQLITE_ROW) {
        int run_id = sqlite3_column_int(run_stmt, 0);
        sql = "SELECT * FROM results WHERE run_id = " + String(run_id) + ";";
        sqlite3_stmt *result_stmt;
        if (sqlite3_prepare_v2(this->db, sql.c_str(), -1, &result_stmt, 0) != SQLITE_OK) {
            DEBUG_SER_PRINT("Failed to prepare statement: ");
            DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
            sqlite3_finalize(result_stmt);
            sysWarn("Failed to cleanup db.");
        }
        if (sqlite3_step(result_stmt) != SQLITE_ROW) {
            DEBUG_SER_PRINTLN("Deleting invalid run: " + String(run_id));
            sql = "DELETE FROM runs WHERE id = " + String(run_id) + ";";
            if (sqlite3_exec(this->db, sql.c_str(), 0, 0, 0) != SQLITE_OK) {
                DEBUG_SER_PRINT("Failed to delete run: ");
                DEBUG_SER_PRINTLN(sqlite3_errmsg(this->db));
                sysWarn("Failed to cleanup db.");
            }
        }
        sqlite3_finalize(result_stmt);
    }
    sqlite3_finalize(run_stmt);
}