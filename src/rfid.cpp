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
void RFID::startReadMulti(void (*callback_function)(uint8_t *)) {
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
    rfid_cmd_stop_multi_t command;
    Serial2.write((uint8_t *) &command, sizeof(command));
}

/**
 * The task that polls the RFID module for tags.
 * @param pv_parameters The callback function to call when a tag is read in the form of a void pointer to comply with the task creation API.
*/
void rfidPollingTask(void *pv_parameters) {
    void (*callback_function)(uint8_t *) = (void (*)(uint8_t *)) pv_parameters;

    uint8_t current_state = 0;
    uint8_t command_state = 0;
    uint8_t data_index = 0;
    uint8_t epc[12];

    while (true) {
        if (Serial2.available() > 0) {
            uint8_t data = Serial2.read();

            if ((data == START_BYTE) && (current_state == 0)) {
                current_state = 1;
            } else if ((current_state == 1) && (data == COMMAND_BYTE) && (command_state == 0)) {
                command_state = 1;
                data_index = 3;
            } else if (command_state == 1) {
                data_index++;
                if ((data_index >= EPC_START_OFFSET) && (data_index <= EPC_END_OFFSET)) {
                    epc[data_index - EPC_START_OFFSET] = data;
                } else if (data_index > EPC_END_OFFSET) {
                    callback_function(epc);
                    current_state = 0;
                    command_state = 0;
                    data_index = 0;
                }
            } else {
                current_state = 0;
                command_state = 0;
                data_index = 0;
            }
        }
    }
}