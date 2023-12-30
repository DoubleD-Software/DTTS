#include <fatal.h>

void sysHalt() {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. System halted.");
    while(1);
}

void sysReset() {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. Restarting...");
    ESP.restart();
}