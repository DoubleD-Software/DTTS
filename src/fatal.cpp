#include <fatal.h>

void sys_halt() {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. System halted.");
    while(1);
}

void sys_reset() {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. Restarting...");
    ESP.restart();
}