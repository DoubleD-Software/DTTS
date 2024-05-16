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
                run_tag_assign_fn = true;
            } else {
                client->close(); // No reason for ws connection = disconnect
            }
        } else if(type == WS_EVT_DISCONNECT) {
            DEBUG_SER_PRINT("Websocket client disconnected: ");
            DEBUG_SER_PRINTLN(client->id());
            run_tag_assign_fn = false;
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

    DEBUG_SER_PRINT("Setting RFID TX-Power:");
    uint8_t set_tx_power[] = {0xAA, 0x00, 0xB6, 0x00, 0x02, 0x07, 0xD0, 0x8F, 0xDD};
    Serial2.write(set_tx_power, sizeof(set_tx_power));
    clearRfidBuf();

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
void RunHandler::setActiveRun(int run_id) {
    active_run_id = run_id;
    run_active = true;
    run_state = RUN_ACTIVE_TAG;
    run_tag_assign_fn = false;
    dtts_armed = false;

    participants = db->getRunParticipants(run_id);
    tag_assignments.clear();
    
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
*/
void RunHandler::handle() {
    ws->cleanupClients();
    if (run_active) {
        if (run_tag_assign_fn) {
            startTagAssignment();
        }
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
        client->text(client_str);
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
            if (!run_tag_assign_fn) return; // Client disconnected
            client->text("2");
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

    DEBUG_SER_PRINTLN("All tags assigned");
    run_tag_assign_fn = false;
    dtts_armed = true;
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