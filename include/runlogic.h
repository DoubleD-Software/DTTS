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

#define RUN_NOT_ACTIVE -1
#define RUN_ACTIVE_TAG 0
#define RUN_ACTIVE 1

typedef struct {
    uint8_t epc[12];
    bool tag_valid;
} RfidEpc;

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
        std::vector<FinisherSprint> finishers;
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
        
        void handleWsData(void *arg, uint8_t *data, size_t len);
        void startTagAssignment();
        void clearRfidBuf(bool print_debug = true, bool wait_data = true);
        RfidEpc readRfidTag(bool block = true);
        void runSprint();
        void calcAndDispTime();
};

#endif