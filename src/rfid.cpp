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
 * @param callbackFunction The function to call when a tag is read. The function has to take a uint8_t pointer as input parameter.
*/
void RFID::startReadMulti(void (*callbackFunction)(uint8_t*)) {
    rfid_cmd_start_multi_t command;
    Serial2.write((uint8_t*)&command, sizeof(command));

    uint8_t cpu_core_task = xPortGetCoreID() ^ 1; // Get the other core than the one this function is running on.

    xTaskCreatePinnedToCore(
        rfidPollingTask, // Function to run on the task
        "RFID Polling", // Task name
        16384, // Stack size in words (twice the amount of bytes)
        (void*) callbackFunction, // Task input parameter (callback function)
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
 * @param pvParameters The callback function to call when a tag is read in the form of a void pointer to comply with the task creation API.
*/
void rfidPollingTask(void *pvParameters) {
    void (*callbackFunction)(uint8_t*) = (void (*)(uint8_t*)) pvParameters; // Cast the void pointer to a function pointer.

    while(1) {
        rfid_tag_resp_t response;

        while(!Serial2.available()); // Wait for the RFID module to respond.

        bool response_success = true;
        for (int i = 0; 1; i++) {
            if (i >= sizeof(response)) { // Prevent buffer overflow.
                response_success = false;
                DEBUG_SER_PRINTLN("RFID module responded with an invalid response to start multi read command. Buffer overflow prevented.");
                return;
            }
            if (Serial2.available()) { // Read the response byte by byte until there is no more data available.
                ((uint8_t*)&response)[i] = Serial2.read();
            } else {
                break;
            }
        }
        if(response.header != 0xAA || response.type != 0x02 || response.command != 0x22) { // Check if the response is a label response.
            response_success = false;
            DEBUG_SER_PRINTLN("Non label response recieved. Callback will not be called.");
        }

        if (response_success) callbackFunction(response.epc); // Call the callback function with the EPC of the tag.
    }
}