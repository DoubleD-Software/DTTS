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
 * @param callback_function The function to call when a tag is read. The function has to take a uint8_t pointer and a uint8_t as parameters for the tag and the RSSI respectively.
*/
void RFID::startReadMulti(void (*callback_function)(uint8_t *, uint8_t)) {
    DEBUG_SER_PRINTLN("Starting RFID multi read...");
    rfid_cmd_start_multi_t command;
    Serial2.write((uint8_t *) &command, sizeof(command));

    uint8_t cpu_core_task = xPortGetCoreID() ^ 1; // Get the other core than the one this function is running on.

    xTaskCreatePinnedToCore(
            rfidPollingTask, // Function to run on the task
            "RFID Polling", // Task name
            16384, // Stack size in words (twice the amount of bytes)
            (void *) callback_function, // Task input parameter (callback function)
            0, // Priority of the task
            &polling_task_handle, // Task handle
            cpu_core_task // CPU core to run the task on
    );
}

/**
 * Stop the RFID module from multi read mode.
*/
void RFID::stopReadMulti() {
    DEBUG_SER_PRINTLN("Stopping RFID multi read...");
    rfid_cmd_stop_multi_t command;
    Serial2.write((uint8_t *) &command, sizeof(command));
}

/**
 * The task that polls the RFID module for tags.
 * @param pv_parameters The callback function to call when a tag is read in the form of a void pointer to comply with the task creation API.
*/
void rfidPollingTask(void *pv_parameters) {
    void (*callback_function)(uint8_t *, uint8_t) = (void (*)(uint8_t *, uint8_t)) pv_parameters; // Cast the void pointer to the callback function pointer.

    uint8_t current_state = 0;
    uint8_t command_state = 0;
    uint8_t data_index = 0;
    uint8_t epc[12]; // The EPC of the tag.
    uint8_t rssi;

    uint32_t last_read = millis(); // The last time a tag was read.

    while (true) {
        if (millis() - last_read > 50000) { // Restart the multi read after 50 seconds 
            DEBUG_SER_PRINTLN("Restarting RFID multi read...");
            rfid_cmd_start_multi_t command;
            Serial2.write((uint8_t *) &command, sizeof(command));
            last_read = millis(); // Update the last read time.
        }

        if (Serial2.available() > 0) { // Check if there is data available on the serial port.
            uint8_t data = Serial2.read(); // Read the data from the serial port.

            if ((data == START_BYTE) && (current_state == 0)) { // Check if the data is the start byte and the current state is 0 = no data received yet.
                current_state = 1;
            } else if ((current_state == 1) && (data == COMMAND_BYTE) && (command_state == 0)) { // Check if the current state is 1 = start byte received and the data is the command byte and the command state is 0 = no command byte received yet.
                command_state = 1;
                data_index = 3;
            } else if (command_state == 1) { // Check if the command state is 1 = command byte received.
                data_index++;
                if (data_index == 6) {
                    rssi = data;
                } else if ((data_index >= EPC_START_OFFSET) && (data_index <= EPC_END_OFFSET)) { // Read data and store it in the EPC array in case the data is part of the EPC.
                    epc[data_index - EPC_START_OFFSET] = data;
                } else if (data_index > EPC_END_OFFSET) { // EPC read, call the callback function and reset the state variables.
                    callback_function(epc, data);
                    current_state = 0;
                    command_state = 0;
                    data_index = 0;
                }
            } else { // No valid data received, reset the state variables.
                current_state = 0;
                command_state = 0;
                data_index = 0;
            }
        }
    }
}