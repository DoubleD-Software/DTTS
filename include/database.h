#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include <sqlite3.h>
#include <debug.h>
#include <fatal.h>
#include <SD.h>
#include <error_codes.h>
#include <vector>

#define SQL_TYPE_INT 0
#define SQL_TYPE_FLOAT 1
#define SQL_TYPE_VC_255 2
#define SQL_TYPE_PRIMARY_KEY 0b1
#define SQL_TYPE_AUTO_INCREMENT 0b10
#define SQL_TYPE_FOREIGN_KEY 0b100

struct SqlColumn {
    String name;
    uint8_t type;
    uint8_t flags;
    String foreign_table;
    String foreign_column;

    SqlColumn(const String& n, const int t, const uint8_t f = 0, const String& ft = "", const String& fc = "") : name(n), type (t), flags(f), foreign_table(ft), foreign_column(fc) {}
};

struct SqlTable {
    String name;
    std::vector<SqlColumn> columns;
    SqlTable(const String& n, const std::vector<SqlColumn>& cols) : name(n), columns(cols) {}
};

typedef struct {
    int type;
    int length;
    String teacher;
    String class_name;
    float avg_grade;
    float avg_time;
} RunInfo;

class Database {
    public:
        Database(String db_path);
        void open();
        void close();

        std::vector<RunInfo> getRunsByDate(int date);

    private:
        void createTables();
        String db_path;
        sqlite3 *db;
        static const SqlTable TABLES[];
};

#endif