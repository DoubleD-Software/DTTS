#include <rfid.h>


RFID::RFID() {   
}

/**
 * Initialize the RFID module. This function has to be called before any other function of this class.
*/
void RFID::begin() {
    DEBUG_SER_PRINTLN("Initializing RFID module...");
    Serial2.begin(RFID_BAUD);

    stopReadMulti(); // Stop any ongoing multi read.
    DEBUG_SER_PRINTLN("RFID module initialized.");
}

/**
 * Start the RFID module in multi read mode.
 * @param callback_function The function to call when a tag is read. The function has to take a uint8_t pointer as input parameter.
*/
void RFID::startReadMulti(void (*callback_function)(uint8_t*)) {
    rfid_cmd_start_multi_t command;
    Serial2.write((uint8_t*)&command, sizeof(command));

    uint8_t cpu_core_task = xPortGetCoreID() ^ 1; // Get the other core than the one this function is running on.

    xTaskCreatePinnedToCore(
        rfidPollingTask, // Function to run on the task
        "RFID Polling", // Task name
        16384, // Stack size in words (twice the amount of bytes)
        (void*) callback_function, // Task input parameter (callback function)
        0, // Priority of the task
        &polling_task_handle, // Task handle
        cpu_core_task // CPU core to run the task on
    );
}

/**
 * Stop the RFID module from multi read mode.
*/
void RFID::stopReadMulti() {
    rfid_cmd_stop_multi_t command;
    Serial2.write((uint8_t*)&command, sizeof(command));

    rfid_resp_stop_multi_t response;
    rfid_resp_stop_multi_t response_ideal;

    uint8_t time_passed = 0;
    while(!Serial2.available()) {
        if (time_passed > RESP_TIMEOUT) {
            DEBUG_SER_PRINTLN("RFID module did not respond to stop multi read command.");
            return;
        }
        delay(1);
        time_passed++;
    }

    Serial2.readBytes((uint8_t*)&response, sizeof(response));
    if (memcmp(&response, &response_ideal, sizeof(response)) != 0) {
        DEBUG_SER_PRINTLN("RFID module responded with an invalid response to stop multi read command.");
        return;
    }
}

/**
 * The task that polls the RFID module for tags.
 * @param pv_parameters The callback function to call when a tag is read in the form of a void pointer to comply with the task creation API.
*/
void rfidPollingTask(void *pv_parameters) {
    void (*callback_function)(uint8_t*) = (void (*)(uint8_t*)) pv_parameters; // Cast the void pointer to a function pointer.

    uint8_t partial_state = 0;
    uint8_t code_state = 0;
    uint8_t data_add = 0;
    bool detected = false;
    uint8_t epc[12];

    while(1) {
        if (Serial2.available() > 0) {
        uint8_t data = Serial2.read();

        // Command code validation
        // Proof of concept code from Chinese man
        if ((data == 0x02) && (partial_state == 0)) {
            partial_state = 1;
        } else if ((partial_state == 1) && (data == 0x22) && (code_state == 0)) {
            code_state = 1;
            data_add = 3;
        } else if (code_state == 1) {
            data_add++;
            if ((data_add >= 9) && (data_add <= 20)) { // Bytes 9 - 21 in response are the EPC code.
                detected = true;
                epc[data_add - 9] = data;
            } else if (data_add >= 21) { // Done reading EPC code, we dont care about the rest of the response.
                data_add = 0;
                partial_state = 0;
                code_state = 0;
                if (detected) { // Call the callback function if a tag was detected.
                    detected = false;
                    callback_function(epc); // Call the callback function with the EPC code as input parameter.
                }
            }
        } else {
            data_add = 0;
            partial_state = 0;
            code_state = 0;
        }
    }
    }
}