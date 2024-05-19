#ifndef RUNLOGIC_H
#define RUNLOGIC_H

#include <Arduino.h>
#include <database.h>
#include <definitions.h>
#include <ESPAsyncWebServer.h>
#include <oled.h>
#include <tm1637.h>
#include <ArduinoJson.h>
#include <vector>
#include <math.h>

#define RUN_NOT_ACTIVE -1
#define RUN_ACTIVE_TAG 0
#define RUN_ACTIVE 1
#define LAP_RUN_LOCK_TIME 5000

typedef struct {
    uint8_t epc[12];
    bool tag_valid;
} RfidEpc;

typedef struct {
    unsigned long lock_start;
    bool locked = false;
} TagLock;

class RunHandler {
    public:
        RunHandler(Database *db, AsyncWebSocket *ws, OLED *oled, TM1637 *num_disp);
        void init();
        void setActiveRun(int run_id, int type);
        int getActiveRunId();
        void handle();

    private:
        Database *db;
        AsyncWebSocket *ws;
        OLED *oled;
        TM1637 *num_disp;
        AsyncWebSocketClient *client;
        std::vector<RunParticipant> participants;
        std::vector<RfidEpc> tag_assignments;
        std::vector<FinisherSprint> sprint_finishers;
        std::vector<FinisherLapRun> lap_run_finishers;
        std::vector<TagLock> tag_locks;
        std::vector<int> lrf_lap_counts;
        PreLapRunInfo pre_lap_run_info;
        String last_ws_rx = "";
        String last_tag_tx = "";

        static const uint8_t read_multi[];
        static const uint8_t stop_multi[];
        uint8_t time_num_buffer[6];
        bool run_active = false;
        bool run_state = RUN_ACTIVE_TAG;
        int active_run_id;
        int active_run_type;
        bool tag_assign_active = false;
        bool ws_data_received = false;
        bool dtts_armed = false;
        bool run_in_progress = false;
        unsigned long run_start_time;
        unsigned long run_time_elapsed;
        int num_lap_run_finishers;
        int lr_total_laps;
        
        void handleWsData(void *arg, uint8_t *data, size_t len);
        void startTagAssignment();
        void clearRfidBuf(bool print_debug = true, bool wait_data = true);
        RfidEpc readRfidTag(bool block = true);
        void runSprint();
        void runLapRun();
        void calcAndDispTimeSprint();
        void calcAndDispTimeLapRun();
};

#endif