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
#define RUN_TYPE_LAP_RUN 0
#define RUN_TYPE_SPRINT 1

#define UNKNOWN_NAME "Unknown"
#define DB_SUCCESS 0
#define DB_FAILED -1
#define DB_CONFLICT -2


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
    int run_id;
    int type;
    int length;
    String teacher;
    String class_name;
    float avg_grade;
    float avg_time;
} RunInfo;

typedef struct {
    int student_id;
    String name;
    float time;
    float grade;
} RunInfoStudent;

typedef struct {
    int type;
    int length;
    int date;
    String teacher;
    String class_name;
    String grading_key_m;
    String grading_key_f;
    float avg_grade;
    float avg_time;
    std::vector<RunInfoStudent> students;
} RunInfoSpecific;

typedef struct {
    int lap_number;
    float length;
    float time;
} RunInfoLap;

typedef struct {
    String student_name;
    float time;
    float grade;
    int length;
    std::vector<RunInfoLap> laps;
} RunInfoStudentLaps;

typedef struct {
    int type;
    int length;
    float grade;
    float time;
    int date;
} StudentInfoRun;

typedef struct {
    String student_name;
    int gender;
    String class_name;
    float sprint_avg_grade;
    float sprint_avg_time;
    float lap_run_avg_grade;
    float lap_run_avg_time;
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
        int putRun(int type, int length, int date, int class_id, int teacher_id, int grading_key_m_id, int grading_key_f_id); // incomplete
        StudentInfo getStudentInfo(int student_id);
        int deleteStudent(int student_id);
        int putStudent(String student_name, int gender, String class_name);

    private:
        void createTables();
        String db_path;
        sqlite3 *db;
        static const SqlTable TABLES[];
};

#endif