#include <fatal.h>
#include <main.h>

/**
 * Halts the system and prints to the serial output if debug is enabled. Additionally, prints error code and message to the OLED display.
 * @param error_code The error code to print.
 * @param msg The message to print.
 * @return This function never returns. A restart is required.
 */
void sysHalt(int error_code, const char *msg) {
    DEBUG_SER_PRINTLN("\nA fatal error has occurred and the system cannot continue. System halted.");
    char oled_str[64] = {0};
    sprintf(oled_str, "ERROR %d: %s\nPlease restart.", error_code, msg);
    oled.clear();
    oled.print(oled_str, 1, WHITE);
    while(1);
}