#ifndef SQL_H
#define SQL_H

#include <Arduino.h>
#include <debug.h>
#include <fatal.h>
#include <sqlite3.h>

class SQL {
    public:
        SQL();
        void begin(const char *db_path);
        void openDB(const char *db_path);
        void closeDB();

    private:
        sqlite3 *db;
};

#endif