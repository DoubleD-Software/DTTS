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
#define SQL_TYPE_REAL 1
#define SQL_TYPE_TEXT 2
#define SQL_TYPE_PRIMARY_KEY 0b1
#define SQL_TYPE_AUTO_INCREMENT 0b10
#define SQL_TYPE_FOREIGN_KEY 0b100
#define RUN_TYPE_LAP_RUN 0
#define RUN_TYPE_SPRINT 1

#define UNKNOWN_NAME "Unknown"
#define DB_SUCCESS 0
#define DB_FAILED -1
#define DB_CONFLICT -2
#define DB_NOT_FOUND -3

struct SqlColumn {
    String name;
    uint8_t type = 0;
    uint8_t flags = 0;
    String foreign_table;
    String foreign_column;

    SqlColumn(const String& n, const int t, const uint8_t f = 0, const String& ft = "", const String& fc = "") : name(n), type (t), flags(f), foreign_table(ft), foreign_column(fc) {}
};

struct SqlTable {
    String name = "";
    std::vector<SqlColumn> columns;
    SqlTable(const String& n, const std::vector<SqlColumn>& cols) : name(n), columns(cols) {}
};

typedef struct {
    int run_id = 0;
    int type = 0;
    int length = 0;
    String teacher;
    String class_name;
    float avg_grade = 0;
    int avg_time = 0;
} RunInfo;

typedef struct {
    int student_id = 0;
    String name;
    int time = 0;
    float grade = 0;
} RunInfoStudent;

typedef struct {
    int type = 0;
    int length = 0;
    int date = 0;
    String teacher;
    String class_name;
    String grading_key_m;
    String grading_key_f;
    float avg_grade = 0;
    int avg_time = 0;
    std::vector<RunInfoStudent> students;
} RunInfoSpecific;

typedef struct {
    int lap_number = 0;
    int length = 0;
    int time = 0;
} RunInfoLap;

typedef struct {
    String student_name;
    int time = 0;
    float grade = 0;
    int length = 0;
    std::vector<RunInfoLap> laps;
} RunInfoStudentLaps;

typedef struct {
    int type = 0;
    int length = 0;
    float grade = 0;
    int time = 0;
    int date = 0;
} StudentInfoRun;

typedef struct {
    String student_name;
    int gender = 0;
    String class_name;
    float sprint_avg_grade = 0;
    int sprint_avg_time = 0;
    float lap_run_avg_grade = 0;
    int lap_run_avg_time = 0;
    std::vector<StudentInfoRun> runs;
} StudentInfo;

class Database {
    public:
        Database(String db_path);
        void open();
        void close();

        std::vector<RunInfo> getRunsByDate(int date);
        RunInfoSpecific getRunInfo(int run_id);
        RunInfoStudentLaps getRunInfoStudent(int run_id, int student_id);
        int deleteRun(int run_id);
        int putRun(int type, int date, int class_id, int grading_key_m_id, int grading_key_f_id, int teacher_id, int length, int laps, std::vector<int> participants);
        StudentInfo getStudentInfo(int student_id);
        int deleteStudent(int student_id);
        int putStudent(String student_name, int gender, int class_id);
        int patchStudent(String student_name, int class_id);
        

    private:
        void createTables();
        String db_path;
        sqlite3 *db;
        static const SqlTable TABLES[];
};

#endif