#include <integrity.h>

void copyFile(String source_path, String destination_path) {
    File source_file = LittleFS.open(source_path, FILE_READ);
    if (!source_file) {
        sysHalt(RECOVERY_FAILED, "Failed to open source file.");
    }

    File dest_file = SD.open(destination_path, FILE_WRITE, true);
    if (!dest_file) {
        sysHalt(SD_FAILED, "SD card is not writable.");
    }

    uint8_t buffer[512];
    int bytes_read;
    while ((bytes_read = source_file.read(buffer, sizeof(buffer))) > 0) {
        dest_file.write(buffer, bytes_read);
    }

    source_file.close();
    dest_file.close();

    DEBUG_SER_PRINTLN("Copied file: " + source_path + " -> " + destination_path);
}

void performIntegrityCheck(String sd_path, String lfs_path) {
    File lfs_dir = LittleFS.open(lfs_path);
    if (!lfs_dir || !lfs_dir.isDirectory()) {
        sysHalt(RECOVERY_FAILED, "Recovery failed.");
    }

    File file = lfs_dir.openNextFile();
    while (file) {
        String sd_file_path = sd_path + "/" + file.name();
        String lfs_file_path = lfs_path + "/" + file.name();

        if (file.isDirectory()) {
            if (!SD.exists(sd_file_path)) {
                SD.mkdir(sd_file_path);
            }
            performIntegrityCheck(sd_file_path, lfs_file_path);
        } else {
            File sd_file = SD.open(sd_file_path, FILE_READ);
            File lfs_file = LittleFS.open(lfs_file_path, "r");

            if (!sd_file) {
                DEBUG_SER_PRINTLN("File missing: " + sd_file_path);
                copyFile(lfs_file_path, sd_file_path);
            } else if (sd_file.size() != lfs_file.size()) {
                DEBUG_SER_PRINTLN("File size mismatch: " + sd_file_path);
                copyFile(lfs_file_path, sd_file_path);
            }

            sd_file.close();
            lfs_file.close();
        }
        file = lfs_dir.openNextFile();
    }
    lfs_dir.close();
}

void deleteExtraneousFiles(String sd_path, String lfs_path) {
    File sd_dir = SD.open(sd_path);
    if (!sd_dir || !sd_dir.isDirectory()) {
        sysHalt(SD_FAILED, "Invalid folder structure.");
    }

    File file = sd_dir.openNextFile();
    while (file) {
        String sd_file_path = sd_path + "/" + file.name();
        String lfs_file_path = lfs_path + "/" + file.name();

        if (file.isDirectory()) {
            deleteExtraneousFiles(sd_file_path, lfs_file_path);
            File sub_dir = SD.open(sd_file_path);
            if (!sub_dir.openNextFile()) {
                DEBUG_SER_PRINTLN("Deleting empty directory from SD card: " + sd_file_path);
                SD.rmdir(sd_file_path);
            }
            sub_dir.close();
        } else {
            if (!LittleFS.exists(lfs_file_path)) {
                DEBUG_SER_PRINTLN("Deleting extraneous file from SD card: " + sd_file_path);
                SD.remove(sd_file_path);
            }
        }
        file = sd_dir.openNextFile();
    }
    sd_dir.close();
}