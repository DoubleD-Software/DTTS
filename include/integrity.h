#ifndef INTEGRITY_H
#define INTEGRITY_H

#include <Arduino.h>
#include <SD.h>
#include <LittleFS.h>
#include <fatal.h>
#include <error_codes.h>

void performIntegrityCheck(String sd_path, String lfs_path);
void deleteExtraneousFiles(String sd_path, String lfs_path);

#endif