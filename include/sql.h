#ifndef SQL_H
#define SQL_H

#include <Arduino.h>
#include <debug.h>
#include <fatal.h>
#include <sqlite3.h>
#include <SD.h>
#include <cstring>

#define SQL_TYPE_INT 0
#define SQL_TYPE_VARCHAR 1

typedef struct {
    const char *name;
    int type;
    int size = 255;
    const char *additional = NULL;
} sql_column_descriptor_t;

typedef struct {
    const char *name;
    int type;
    int value_int;
    char *value_varchar;
} sql_column_t;

class SQL {
    public:
        SQL();
        void begin(const char *db_path);
        void openDB(const char *db_path);
        void closeDB();
        int clearTable(const char *table_name);
        int dropTable(const char *table_name);
        int createTable(const char *table_name, sql_column_descriptor_t *columns, int column_count);
        int insertIntoTable(const char *table_name, sql_column_t *columns, int column_count);
        void getValueFromTable(const char *table_name, sql_column_t *column, sql_column_t *where);
        void getValuesFromTable(const char *table_name, sql_column_t *columns, int column_count, sql_column_t *where);

    private:
        int dbExecSimple(const char *sql);
        sqlite3 *db;
};

#endif