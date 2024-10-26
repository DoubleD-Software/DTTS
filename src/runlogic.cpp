#include <runlogic.h>

const uint8_t RunHandler::read_multi[] = {0xAA, 0x00, 0x27, 0x00, 0x03, 0x22, 0xFF, 0xFF, 0x4A, 0xDD};
const uint8_t RunHandler::stop_multi[] = {0xAA, 0x00, 0x28, 0x00, 0x00, 0x28, 0xDD};

/**
 * Constructor for the RunHandler class.
 * @param db The database object to use for the handler.
 * @param server The server object to use for the handler.
*/
RunHandler::RunHandler(Database *db, AsyncWebSocket *ws, OLED *oled, TM1637 *num_disp) {
    this->db = db;
    this->ws = ws;
    this->oled = oled;
    this->num_disp = num_disp;
}

/**
 * Initializes the run handler by setting up the websocket and registering the run logic handler.
*/
void RunHandler::init() {
    DEBUG_SER_PRINTLN("Initializing run handler...");

    ws->onEvent([&](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if(type == WS_EVT_CONNECT) {
            DEBUG_SER_PRINT("Websocket client connected: ");
            DEBUG_SER_PRINT(client->id());
            DEBUG_SER_PRINT(", ");
            DEBUG_SER_PRINTLN(client->remoteIP().toString());
            this->client = client;
            if (getActiveRunId() == RUN_ACTIVE_TAG) {
                tag_assign_active = true;                       // When a client connects the tag assignment is started
                if (last_tag_tx != "") {
                    ws->textAll(last_tag_tx);
                }
            } else if (getActiveRunId() != RUN_NOT_ACTIVE) {
                if (run_in_progress) {
                    String client_str = "1 " + String(run_time_elapsed);
                    ws->textAll(client_str);
                }
            } else {
                ws->textAll("4");
                client->close(); // No reason for ws connection, send 4 to indicate the end of a run in case there is still an active one
            }
        } else if(type == WS_EVT_DISCONNECT) {
            DEBUG_SER_PRINT("Websocket client disconnected: ");
            DEBUG_SER_PRINTLN(client->id());
        } else if(type == WS_EVT_ERROR) {
            DEBUG_SER_PRINTLN("Websocket error.");
        } else if(type == WS_EVT_DATA) {
            handleWsData(arg, data, len);
        }
    });

    // Init rfid module
    Serial2.begin(RFID_BAUD);

    DEBUG_SER_PRINT("Reading RFID HW version:");
    uint8_t read_hw[] = {0xAA, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0xDD};
    Serial2.write(read_hw, sizeof(read_hw));
    clearRfidBuf();
    
    DEBUG_SER_PRINT("Reading RFID SW version:");
    uint8_t read_sw[] = {0xAA, 0x00, 0x03, 0x00, 0x01, 0x01, 0x05, 0xDD};
    Serial2.write(read_sw, sizeof(read_sw));
    clearRfidBuf();
    
    DEBUG_SER_PRINT("Setting RFID Work-Area:");
    uint8_t set_wa[] = {0xAA, 0x00, 0x07, 0x00, 0x01, 0x03, 0x0B, 0xDD};
    Serial2.write(set_wa, sizeof(set_wa));
    clearRfidBuf();

    // DEBUG_SER_PRINT("Setting RFID TX-Power:");
    // uint8_t set_tx_power[] = {0xAA, 0x00, 0xB6, 0x00, 0x02, 0x07, 0xD0, 0x8F, 0xDD};
    // Serial2.write(set_tx_power, sizeof(set_tx_power));
    // clearRfidBuf();

    DEBUG_SER_PRINT("Setting RFID Sens-Mode:");
    uint8_t set_sens[] = {0xAA, 0x00, 0xF5, 0x00, 0x01, 0x00, 0xF6, 0xDD};
    Serial2.write(set_sens, sizeof(set_sens));
    clearRfidBuf();

    DEBUG_SER_PRINTLN("Initialized run handler.");
}

/**
 * Sets the active run for the handler which then retrieves the participants for the run.
 * @param run_id The id of the run to set as active.
*/
void RunHandler::setActiveRun(int run_id, int type) {
    active_run_id = run_id;
    active_run_type = type;
    run_active = true;
    run_state = RUN_ACTIVE_TAG;
    tag_assign_active = false;
    dtts_armed = false;
    run_in_progress = false;
    last_tag_tx = "";

    participants = db->getRunParticipants(run_id);
    tag_assignments.clear();
    sprint_finishers.clear();
    lap_run_finishers.clear();
    lrf_lap_counts.clear();
    tag_locks.clear();
    num_lap_run_finishers = 0;

    DEBUG_SER_PRINT("Active run set to: ");
    DEBUG_SER_PRINTLN(run_id);
}

/**
 * Gets the id of the active run.
 * @return The id of the active run.
*/
int RunHandler::getActiveRunId() {
    if ((run_state == RUN_ACTIVE_TAG) && (run_active == true)) {
        return RUN_ACTIVE_TAG;
    } else if ((run_state == RUN_ACTIVE) && (run_active == true)) {
        return active_run_id;
    } else {
        return RUN_NOT_ACTIVE;
    }
}

/**
 * Handles the run logic by checking the state of the run and performing the necessary actions.
 * This function should be called from the main loop through the server object.
 * When a run is armed, this function blocks until the run is complete and also waits for the start button press.
*/
void RunHandler::handle() {
    ws->cleanupClients();
    if (run_active) {
        update_idle_disp = true;
        if (dtts_armed) {
            // Do some pre-calculations for a lap run
            if (active_run_type == RUN_TYPE_LAP_RUN) {
                pre_lap_run_info = db->getPreLapRunInfo(active_run_id);
                double int_part;
                double frac_part = modf(pre_lap_run_info.laps, &int_part);
                int full_lap_length = pre_lap_run_info.length / pre_lap_run_info.laps;
                double partial_lap_length = pre_lap_run_info.length - (full_lap_length * int_part);

                lr_total_laps = int_part;
                if (partial_lap_length > 0) {
                    lr_total_laps++;
                }

                for (int i = 0; i < participants.size(); i++) {
                    FinisherLapRun finisher;
                    finisher.student_id = participants[i].student_id;
                    finisher.student_name = participants[i].student_name;

                    if (partial_lap_length > 0) { // The first lap is a partial lap when the teacher entered a real number of laps
                        FinisherLap lap;
                        lap.length = partial_lap_length;
                        finisher.laps.push_back(lap);
                    }
                    for (int j = 0; j < int_part; j++) {
                        FinisherLap lap;
                        lap.length = full_lap_length;
                        finisher.laps.push_back(lap);
                    }

                    lap_run_finishers.push_back(finisher);
                    int lap_count = 0;
                    lrf_lap_counts.push_back(lap_count);

                    TagLock lock;
                    tag_locks.push_back(lock);
                }
            }

            while (digitalRead(BUTTON_PIN) == HIGH) {
                if (ws_data_received && last_ws_rx == "0") {
                    ws->textAll("3");
                    ws_data_received = false;
                    ws->closeAll();
                    oled->clear();
                    oled->print("Lauf\ngestoppt!", 2);
                    num_disp->displayString("Fertig");
                    run_active = false;
                    db->deleteRun(active_run_id);
                    delay(10000);
                    return;
                }
            }
            DEBUG_SER_PRINT("Starting RFID multi read:");
            Serial2.write(read_multi, sizeof(read_multi));
            clearRfidBuf();
            run_in_progress = true;
            oled->clear();
            oled->print("Lauf\ngestartet!", 2);
            ws->textAll("1 0");
            if (active_run_type == RUN_TYPE_SPRINT) {
                runSprint();
            } else {
                runLapRun();
            }
        } else if (tag_assign_active) {
            startTagAssignment();
        }
    } else if (update_idle_disp) {
        update_idle_disp = false;
        oled->clear();
        oled->print("DTTS\n", 2);
        oled->print("System betriebsbereit\n", 1);
        oled->print("WLAN aktiv", 1);
        num_disp->displayString(" DTTS ");
    }
}

/**
 * Handles incoming websocket data.
 * @param arg The websocket client that sent the data.
 * @param data The data sent by the client.
 * @param len The length of the data.
*/
void RunHandler::handleWsData(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        String data_str = (const char *) data;
        DEBUG_SER_PRINT("Received data: ");
        DEBUG_SER_PRINTLN(data_str);
        last_ws_rx = data_str;
        ws_data_received = true;
    }
}

/**
 * Starts the tag assignment process for the latest created run
 * @param client The websocket client to communicate with.
*/
void RunHandler::startTagAssignment() {
    DEBUG_SER_PRINTLN("Starting tag assignment...");

    oled->clear();
    oled->print("Tags\nzuordnen..", 2);

    DEBUG_SER_PRINT("Starting RFID multi read:");
    Serial2.write(read_multi, sizeof(read_multi));
    clearRfidBuf();

    int participant_number = tag_assignments.size();
    if (participant_number == participants.size()) {
        DEBUG_SER_PRINTLN("All tags assigned.");
        return;
    }

    bool last_tag = false;

    for (; participant_number < participants.size(); participant_number++) {
        if (participant_number == participants.size() - 1) {
            last_tag = true;
        }

        String client_str;
        if (last_tag) {
            client_str = "1 ";
        } else {
            client_str = "0 ";
        }

        client_str += String(participants[participant_number].student_name);
        last_tag_tx = client_str;
        ws->textAll(client_str);
        while (true) {
            RfidEpc tag = readRfidTag(true);
            bool tag_collision = false;
            for (int i = 0; i < tag_assignments.size(); i++) {
                if (memcmp(tag.epc, tag_assignments[i].epc, 12) == 0) {
                    DEBUG_SER_PRINTLN("Tag already assigned. Skipping...");
                    tag_collision = true;
                    break;
                }
            }
            if (!tag_collision) {
                tag_assignments.push_back(tag);
                break;
            }
        }

        while (true) {
            ws->textAll("2");
            while(!ws_data_received) {
                delay(10);
            }
            ws_data_received = false;
            if (!last_tag) {
                if (last_ws_rx == "0") {
                    break;
                } else {
                    DEBUG_SER_PRINTLN("Invalid client response. Expected 0");
                }
            } else {
                if (last_ws_rx == "1") {
                    break;
                } else {
                    DEBUG_SER_PRINTLN("Invalid client response. Expected 1");
                }
            }
        }
    }

    DEBUG_SER_PRINTLN("All tags assigned, DTTS is armed");

    // DTTS is armed after all tags are assigned, this means that when the start button is pressed, the run will start
    tag_assign_active = false;
    dtts_armed = true;
    run_state = RUN_ACTIVE;
    ws->closeAll();
    num_disp->displayString("Scharf");

    clearRfidBuf(false, false);
    Serial2.write(stop_multi, sizeof(stop_multi));
    DEBUG_SER_PRINT("Stopping RFID multi read:");
    clearRfidBuf();

    oled->clear();
    oled->print("System\nscharf!", 2);
}

/**
 * Runs the sprint logic for the active sprint run.
 * This function blocks until the run is complete.
*/
void RunHandler::runSprint() {
    DEBUG_SER_PRINTLN("Sprint run started.");
    run_start_time = millis();
    unsigned long last_ws_send = millis();
    unsigned long last_time_disp = millis();

    int participants_size_before = participants.size();

    while(true) {
        if (sprint_finishers.size() == participants_size_before) {
            break;
        }

        if ((millis() - last_time_disp) > 5) { // Update display every 5ms even thought only 10ms are required, compensating for the time it takes to calculate finisher times
            last_time_disp = millis();
            calcAndDispTimeSprint();
        }
        RfidEpc tag = readRfidTag(false);
        if (tag.tag_valid) {
            for (int i = 0; i < tag_assignments.size(); i++) {
                if (memcmp(tag.epc, tag_assignments[i].epc, 12) == 0) {
                    FinisherSprint finisher;
                    finisher.student_id = participants[i].student_id;
                    finisher.time = run_time_elapsed;
                    finisher.student_name = participants[i].student_name;
                    sprint_finishers.push_back(finisher);
                    tag_assignments.erase(tag_assignments.begin() + i);
                    String student_name = participants[i].student_name;
                    participants.erase(participants.begin() + i);

                    char time_str[10]; // 00:00:00\n
                    snprintf(time_str, 10, "%02d:%02d:%02d\n", (finisher.time / 1000) / 60, (finisher.time / 1000) % 60, (finisher.time % 1000) / 10);

                    oled->clear();
                    oled->print(time_str, 2);
                    oled->print(student_name.c_str(), 1);
                    break;
                }
            }
        }
        if ((millis() - last_ws_send) > 1000) {
            String client_str = "1 " + String(run_time_elapsed);
            ws->textAll(client_str);

            JsonDocument finishers_json;
            for (int i = 0; i < sprint_finishers.size(); i++) {
                String index = String(i);
                finishers_json["finished"][index]["name"] = sprint_finishers[i].student_name;
                finishers_json["finished"][index]["time"] = sprint_finishers[i].time;
            }
            String finishers_str;
            serializeJson(finishers_json, finishers_str);
            if (finishers_str == "null") {
                finishers_str = "{}";
            }
            finishers_str = "2 " + finishers_str;
            ws->textAll(finishers_str);
            
            last_ws_send = millis();
        }
        if (ws_data_received && last_ws_rx == "0") {
            ws->textAll("3");
            ws_data_received = false;
            ws->closeAll();
            oled->clear();
            oled->print("Lauf\ngestoppt!", 2);
            num_disp->displayString("Fertig");
            run_active = false;
            db->deleteRun(active_run_id);

            clearRfidBuf(false, false);
            Serial2.write(stop_multi, sizeof(stop_multi));
            DEBUG_SER_PRINT("Stopping RFID multi read:");
            clearRfidBuf();

            delay(10000);
            return;
        }
    }
    String client_str = "4 " + String(active_run_id);
    ws->textAll(client_str);
    ws->closeAll();
    oled->clear();
    oled->print("Lauf\nbeendet!", 2);
    num_disp->displayString("Fertig");

    db->insertSprintResults(active_run_id, sprint_finishers);
    run_active = false;

    clearRfidBuf(false, false);
    Serial2.write(stop_multi, sizeof(stop_multi));
    DEBUG_SER_PRINT("Stopping RFID multi read:");
    clearRfidBuf();

    delay(10000);
}

/**
 * Runs the lap run logic for the active lap run.
 * This function blocks until the run is complete.
*/
void RunHandler::runLapRun() {
    DEBUG_SER_PRINTLN("Lap run started.");
    run_start_time = millis();
    unsigned long last_ws_send = millis();
    unsigned long last_time_disp = millis();

    while(true) {
        if (num_lap_run_finishers == participants.size()) {
            break;
        }

        if ((millis() - last_time_disp) > 5) { // Update display every 5ms even thought only 10ms are required, compensating for the time it takes to calculate finisher times
            last_time_disp = millis();
            calcAndDispTimeLapRun();
        }
        calcAndDispTimeLapRun();
        RfidEpc tag = readRfidTag(false);
        if (tag.tag_valid) {
            for (int i = 0; i < tag_assignments.size(); i++) {
                if (memcmp(tag.epc, tag_assignments[i].epc, 12) == 0) {
                    if (!tag_locks[i].locked) {
                        tag_locks[i].locked = true;
                        tag_locks[i].lock_start = millis();
                    } else {
                        break;
                    }

                    int current_lap_time = run_time_elapsed;
                    for (int j = 0; j < lrf_lap_counts[i]; j++) {
                        current_lap_time -= lap_run_finishers[i].laps[j].time;
                    }
                    lap_run_finishers[i].laps[lrf_lap_counts[i]].time = current_lap_time;
                    lrf_lap_counts[i]++;
                    if (lrf_lap_counts[i] == lr_total_laps) {   // Student has finished all laps of the run
                        int total_time = 0;
                        for (int j = 0; j < pre_lap_run_info.laps; j++) {
                            total_time += lap_run_finishers[i].laps[j].time;
                        }
                        lap_run_finishers[i].total_time = total_time;
                        num_lap_run_finishers++;

                        char time_str[10]; // 00:00:00\n
                        snprintf(time_str, 10, "%02d:%02d:%02d\n", (total_time / 1000) / 3600, (total_time / 1000) / 60, (total_time / 1000) % 60);

                        oled->clear();
                        oled->print(time_str, 2);
                        oled->print(participants[i].student_name.c_str(), 1);
                    }
                    break;
                }
            }
        }
        if (millis() - last_ws_send > 1000) {
            String client_str = "1 " + String(run_time_elapsed);
            ws->textAll(client_str);

            JsonDocument finishers_json;
            for (int i = 0; i < lap_run_finishers.size(); i++) {
                if (lap_run_finishers[i].total_time == 0) continue;

                String index = String(i);
                finishers_json["finished"][index]["name"] = lap_run_finishers[i].student_name;
                finishers_json["finished"][index]["time"] = lap_run_finishers[i].total_time;
            }
            String finishers_str;
            serializeJson(finishers_json, finishers_str);
            if (finishers_str == "null") {
                finishers_str = "{}";
            }
            finishers_str = "2 " + finishers_str;
            ws->textAll(finishers_str);
            
            last_ws_send = millis();
        }
        if (ws_data_received && last_ws_rx == "0") {
            ws->textAll("3");
            ws_data_received = false;
            ws->closeAll();
            oled->clear();
            oled->print("Lauf\ngestoppt!", 2);
            num_disp->displayString("Fertig");
            run_active = false;
            db->deleteRun(active_run_id);

            clearRfidBuf(false, false);
            Serial2.write(stop_multi, sizeof(stop_multi));
            DEBUG_SER_PRINT("Stopping RFID multi read:");
            clearRfidBuf();

            delay(10000);
            return;
        }
        for (int i = 0; i < tag_locks.size(); i++) {
            if (tag_locks[i].locked && (millis() - tag_locks[i].lock_start >= LAP_RUN_LOCK_TIME)) {
                tag_locks[i].locked = false;
            }
        }
    }
    String client_str = "4 " + String(active_run_id);
    ws->textAll(client_str);
    ws->closeAll();
    oled->clear();
    oled->print("Lauf\nbeendet!", 2);
    num_disp->displayString("Fertig");
    db->insertLapRunResults(active_run_id, lap_run_finishers);
    run_active = false;

    clearRfidBuf(false, false);
    Serial2.write(stop_multi, sizeof(stop_multi));
    DEBUG_SER_PRINT("Stopping RFID multi read:");
    clearRfidBuf();

    delay(10000);    
}

/**
 * Calculates the time elapsed since the run started and displays it on the 6-digit display MM:SS::MS
*/
void RunHandler::calcAndDispTimeSprint() {
    unsigned long current_time = millis();
    run_time_elapsed = current_time - run_start_time;
    int time_seconds = run_time_elapsed / 1000;
    int time_minutes = time_seconds / 60;
    time_seconds = time_seconds % 60;
    int time_millis = (run_time_elapsed % 1000) / 10;

    time_num_buffer[0] = time_minutes / 10;
    time_num_buffer[1] = (time_minutes % 10) | DISPLAY_DOT;
    time_num_buffer[2] = time_seconds / 10;
    time_num_buffer[3] = (time_seconds % 10) | DISPLAY_DOT;
    time_num_buffer[4] = time_millis / 10;
    time_num_buffer[5] = time_millis % 10;

    num_disp->displayNumBuffer(time_num_buffer);
}

/**
 * Calculates the time elapsed since the run started and displays it on the 6-digit display HH:MM:SS
*/
void RunHandler::calcAndDispTimeLapRun() {
    unsigned long current_time = millis();
    run_time_elapsed = current_time - run_start_time;
    int time_seconds = run_time_elapsed / 1000;
    int time_minutes = time_seconds / 60;
    time_seconds = time_seconds % 60;
    int time_hours = time_minutes / 60;
    time_minutes = time_minutes % 60;

    time_num_buffer[0] = time_hours / 10;
    time_num_buffer[1] = (time_hours % 10) | DISPLAY_DOT;
    time_num_buffer[2] = time_minutes / 10;
    time_num_buffer[3] = (time_minutes % 10) | DISPLAY_DOT;
    time_num_buffer[4] = time_seconds / 10;
    time_num_buffer[5] = time_seconds % 10;

    num_disp->displayNumBuffer(time_num_buffer);
}

/**
 * Clears the RFID buffer
*/
void RunHandler::clearRfidBuf(bool print_debug, bool wait_data) {
    while (!Serial2.available() && wait_data) {
        delay(10);
    }
    while(Serial2.available()) {
        volatile uint8_t data = Serial2.read();
        if (print_debug) {
            DEBUG_SER_PRINT(" ");
            DEBUG_SER_PRINT(data, HEX);
        }
    }
    if (print_debug) DEBUG_SER_PRINTLN();
}

/**
 * Reads an RFID tag from the RFID module. The multi read has to be started before calling this function.
 * @param block Whether to block until a valid tag is read.
 * @return The EPC of the tag read.
*/
RfidEpc RunHandler::readRfidTag(bool block) {
    RfidEpc epc;
    epc.tag_valid = false;

    uint8_t serial_buffer[64] = {0};

    clearRfidBuf(false, false);

    while (true) {
        while (Serial2.available() < 8);
        int bytes_read = Serial2.readBytesUntil(0xDD, serial_buffer, 64);
        serial_buffer[bytes_read] = 0xDD;
        if ((serial_buffer[0] == 0xAA) && (serial_buffer[1] = 0x02) && (serial_buffer[2] == 0x22) && (bytes_read == 23)) {
            epc.tag_valid = true;
            for (int i = 0; i < 12; i++) {
                epc.epc[i] = serial_buffer[i + 8];
            }
            break;
        } else if (!block) {
            break;
        }
    }

    return epc;
}