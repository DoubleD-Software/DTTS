#ifndef DATABASE_H
#define DATABASE_H

#include <Arduino.h>
#include <sqlite3.h>
#include <debug.h>
#include <fatal.h>
#include <SD.h>
#include <error_codes.h>
#include <definitions.h>
#include <vector>

#define SQL_TYPE_INT 0
#define SQL_TYPE_REAL 1
#define SQL_TYPE_TEXT 2
#define SQL_TYPE_PRIMARY_KEY 0b1
#define SQL_TYPE_AUTO_INCREMENT 0b10
#define SQL_TYPE_FOREIGN_KEY 0b100
#define RUN_TYPE_LAP_RUN 1
#define RUN_TYPE_SPRINT 0
#define GENDER_TYPE_MALE 0
#define GENDER_TYPE_FEMALE 1

#define UNKNOWN_NAME "N/A"
#define DB_SUCCESS 0
#define DB_FAILED -1
#define DB_CONFLICT -2
#define DB_NOT_FOUND -3
#define DB_INVALID -4

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
    double avg_grade = 0;
    int avg_time = 0;
} RunInfo;

typedef struct {
    int student_id = 0;
    String name;
    int time = 0;
    double grade = 0;
    int gender = 0;
} RunInfoStudent;

typedef struct {
    int type = 0;
    int length = 0;
    int date = 0;
    String teacher;
    String class_name;
    String grading_key_m;
    String grading_key_f;
    double avg_grade = 0;
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
    double grade = 0;
    int length = 0;
    std::vector<RunInfoLap> laps;
} RunInfoStudentLaps;

typedef struct {
    int type = 0;
    int length = 0;
    double grade = 0;
    int time = 0;
    int date = 0;
    int run_id = 0;
} StudentInfoRun;

typedef struct {
    String student_name;
    int gender = 0;
    String class_name;
    double global_avg_grade = 0;
    double sprint_avg_grade = 0;
    int sprint_avg_time = 0;
    double lap_run_avg_grade = 0;
    int lap_run_avg_time = 0;
    std::vector<StudentInfoRun> runs;
} StudentInfo;

typedef struct {
    String name;
    int type = 0;
    int min_time = 0;
    int length = 0;
    int gender = 0;
    int id = 0;
} GradingKeySimple;

typedef struct {
    double grade = 0;
    int time = 0;
} GradingKeyGrade;

typedef struct {
    String name;
    int type = 0;
    int length = 0;
    int gender = 0;
    std::vector<GradingKeyGrade> grades;
} GradingKey;

typedef struct {
    String name;
    int id = 0;
} GradingKeyIdMap;

typedef struct {
    std::vector<GradingKeyIdMap> males;
    std::vector<GradingKeyIdMap> females;
} GradingKeyMap;

typedef struct {
    int class_id = 0;
    String name;
    int size = 0;
    double sprint_avg_grade = 0;
    int sprint_avg_time = 0;
    double lap_run_avg_grade = 0;
    int lap_run_avg_time = 0;
} ClassInfoSimple;

typedef struct {
    int run_id = 0;
    int type = 0;
    int length = 0;
    int date = 0;
    double avg_grade = 0;
    double avg_time = 0;
} ClassInfoRun;

typedef struct {
    int student_id = 0;
    String name;
    int gender;
    double avg_grade = 0;
} ClassInfoStudent;

typedef struct {
    String name;
    double global_avg_grade = 0;
    double sprint_avg_grade = 0;
    int sprint_avg_time = 0;
    double lap_run_avg_grade = 0;
    int lap_run_avg_time = 0;
    std::vector<ClassInfoRun> runs;
    std::vector<ClassInfoStudent> students;
} ClassInfo;

typedef struct {
    int student_id = 0;
    String name;
} ClassStudent;

typedef struct {
    int class_id = 0;
    String name;
} ClassName;

typedef struct {
    int teacher_id = 0;
    String name;
} TeacherSimple;

typedef struct {
    String name;
    String username;
} Teacher;

typedef struct {
    int type = 0;
    int length = 0;
    int date = 0;
    String class_name;
    String grading_key_m;
    String grading_key_f;
    String teacher_name;
} RunInfoActive;

typedef struct {
    int student_id = 0;
    String student_name;
} RunParticipant;

typedef struct {
    int access_level = -1;
    int user_id = 0;
} UserAuth;

typedef struct {
    int student_id = 0;
    String student_name;
    int time = 0;
} FinisherSprint;

class Database {
    public:
        Database(String db_path);
        void open();
        void close();

        std::vector<RunInfo> getRunsByDate(int date);
        RunInfoSpecific getRunInfo(int run_id);
        RunInfoStudentLaps getRunInfoStudent(int run_id, int student_id);
        int deleteRun(int run_id);
        int putRun(int type, int date, int class_id, int grading_key_m_id, int grading_key_f_id, int teacher_id, int length, double laps, std::vector<int> participants);
        StudentInfo getStudentInfo(int student_id);
        int deleteStudent(int student_id);
        int putStudent(String name, int gender, int class_id);
        int patchStudent(int id, String name, int class_id);
        std::vector<GradingKeySimple> getGradingKeys();
        GradingKey getGradingKey(int grading_key_id);
        GradingKeyMap getGradingKeyMap(int type, int length);
        int deleteGradingKey(int grading_key_id);
        int putGradingKey(String name, int type, int length, int gender, std::vector<GradingKeyGrade> grades);
        int patchGradingKey(int id, String name, int length, std::vector<GradingKeyGrade> grades);
        std::vector<ClassInfoSimple> getClasses();
        ClassInfo getClassInfo(int class_id);
        int deleteClass(int class_id);
        int putClass(String name);
        int patchClass(int id, String name);
        std::vector<ClassStudent> getClassStudents(int class_id);
        std::vector<ClassName> getClassNames();
        std::vector<TeacherSimple> getTeachers();
        Teacher getTeacher(int teacher_id);
        int deleteTeacher(int teacher_id);
        int putTeacher(String name, String username, String password);
        int patchTeacher(int id, String name, String username, String password);
        UserAuth checkPassword(String username, String password);
        RunInfoActive getRunInfoActive(int run_id);
        std::vector<RunParticipant> getRunParticipants(int run_id);
        int insertSprintResults(int run_id, std::vector<FinisherSprint> finishers);

    private:
        void createTables();
        String db_path;
        sqlite3 *db;
        static const SqlTable TABLES[];
};

#endif