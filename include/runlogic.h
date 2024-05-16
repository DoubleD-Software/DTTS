#ifndef RUNLOGIC_H
#define RUNLOGIC_H

#include <Arduino.h>
#include <database.h>
#include <definitions.h>
#include <ESPAsyncWebServer.h>
#include <oled.h>
#include <tm1637.h>

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
        void setActiveRun(int run_id);
        int getActiveRunId();
        void handle();

    private:
        Database *db;
        AsyncWebSocket *ws;
        OLED *oled;
        TM1637 *num_disp;
        bool run_active = false;
        bool run_state = RUN_ACTIVE_TAG;
        int active_run_id;
        std::vector<RunParticipant> participants;
        std::vector<RfidEpc> tag_assignments;
        String last_ws_rx = "";
        static const uint8_t read_multi[];
        static const uint8_t stop_multi[];
        bool run_tag_assign_fn = false;
        bool ws_data_received = false;
        bool dtts_armed = false;
        AsyncWebSocketClient *client;
        void handleWsData(void *arg, uint8_t *data, size_t len);
        void startTagAssignment();
        void clearRfidBuf(bool print_debug = true, bool wait_data = true);
        RfidEpc readRfidTag(bool block = true);
};

#endif