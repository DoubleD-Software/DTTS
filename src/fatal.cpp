#include <fatal.h>

/**
 * Halts the system and prints to the serial output if debug is enabled.
 */
void sysHalt() {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. System halted.");
    while(1);
}

/**
 * Resets the system and prints to the serial output if debug is enabled.
 */
void sysReset() {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. Restarting...");
    ESP.restart();
}