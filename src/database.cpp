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
                sql += ", FOREIGN KEY(" + String(column.name) + ") REFERENCES " + column.foreign_table + "(" + column.foreign_column + ")";
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
    // TODO: Implement this function
    return runs;
}